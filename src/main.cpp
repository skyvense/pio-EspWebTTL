
/*
    Esp WebTTL 
    Copyright (c) 2022 Nate Zhang (skyvense@gmail.com)
    
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
    
    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.
    
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/


#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#else
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EasyLed.h>
#include <TimeLib.h>
#include <SPI.h>
#include <SD.h>
#include <CircularBuffer.h>

CircularBuffer<uint8_t,2000> cached_screen_bytes; //A typical telnet screen is 80*25=2000

const int chipSelect = D8;
File record_file;
int record_file_opened = 0; //try once only at setup()
File root;

#define STATUS_LED  2
EasyLed led(STATUS_LED, EasyLed::ActiveLevel::Low, EasyLed::State::On);  //Use this for an active-low LED
//EasyLed led(STATUS_LED, EasyLed::ActiveLevel::High, EasyLed::State::On);  //Use this for an active-high LED

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define MAX_SRV_CLIENTS 5
WiFiServer telnet_server(23);
WiFiClient serverClients[MAX_SRV_CLIENTS];
AsyncWebServer web(80);
AsyncWebSocket ws("/ws");

int last_srv_clients_count = 0;
int flashing_ip = 0;
time_t last_active_time = 0;
int has_active = 0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
void BaseConfig();

struct EMPTY_SERIAL
{
  void println(const char *){}
  void println(String){}
  void printf(const char *, ...){}
  void print(const char *){}
  //void print(Printable) {}
  void begin(int){}
  void end(){}
} _EMPTY_SERIAL;
#define Serial_debug  _EMPTY_SERIAL
//#define Serial_debug  Serial

void WriteSDFileRecord(uint8_t *buf, size_t len)
{
  if (record_file_opened == 1)
  {
    record_file.write(buf, len);
    record_file.flush();
  }
}


//Wifi functions---------------------------------------------------------------
struct Config {
  String SSID = "S1";
  String Passwd = "abc";
  String Server = "192.168.8.1";
  String Token = "0000";
};
Config _config;

//Load Wifi config from spi fs
bool LoadConfig()
{
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile)
  {
    Serial_debug.println("Failed to open config file");
    return false;
  }
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, configFile);
  if (error)
  {
    Serial_debug.println("Failed to read file, using default configuration");
    return false;
  }
  _config.SSID = doc["SSID"] | "fail";
  _config.Passwd = doc["Passwd"] | "fail";
  if (_config.SSID == "fail" || _config.Passwd == "fail")
  {
    return false;
  }
  else
  {
    Serial_debug.println("Load wifi config from spiffs successful.");
    Serial_debug.print("Loaded ssid: ");
    Serial_debug.println(_config.SSID);
    Serial_debug.print("Loaded passwd: ");
    Serial_debug.println(_config.Passwd);
    return true;
  }
}

//save wifi config to fs
bool SaveConfig()
{
  DynamicJsonDocument doc(1024);
  JsonObject root = doc.to<JsonObject>();
  root["SSID"] = _config.SSID;
  root["Passwd"] = _config.Passwd;
  root["Server"] = _config.Server;
  root["Token"] = _config.Token;

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile)
  {
    Serial_debug.println("Failed to open config file for writing");
    return false;
  }
  if (serializeJson(doc, configFile) == 0)
  {
    Serial_debug.println("Failed to write to file");
    return false;
  }
  return true;
}


void SmartConfig()
{
  Serial_debug.println("Use smart config to connect wifi.");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Wifi SmartConfig");
  WiFi.mode(WIFI_STA);
  WiFi.beginSmartConfig();
  while (1)
  {
    Serial_debug.println("Wait to connect wifi...");
    led.flash(10, 50, 50, 0, 0);
    display.print(".");
    display.display();
    delay(1000);
    if (WiFi.smartConfigDone())
    {
      Serial_debug.println("WiFi connected by smart config.");
      Serial_debug.print("SSID:");
      Serial_debug.println(WiFi.SSID());
      Serial_debug.print("IP Address:");
      Serial_debug.println(WiFi.localIP().toString());
      display.print("Smartconfig connected.\nIP:");
      display.print(WiFi.localIP());
      display.println("");
      display.print("AP: ");
      display.println(WiFi.SSID());
      display.display();

      _config.SSID = WiFi.SSID();
      _config.Passwd = WiFi.psk();
      if (!SaveConfig())
      {
        Serial_debug.println("Failed to save config");
      }
      else
      {
        Serial_debug.println("Config saved");
      }
      break;
    }
  }
}


//Connect wifi
void ConnectWifi()
{
  if (LoadConfig())
  {
    BaseConfig();
  }
  else
  {
    SmartConfig();
  }
}

bool WiFiWatchDog()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    BaseConfig();
  }
  return true;
}

void BaseConfig()
{
  Serial_debug.println("Use base config to connect wifi.");
  led.flash(4, 125, 125, 0, 0);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Connecting ");
  display.print(_config.SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(_config.SSID, _config.Passwd);
  //连接超时时间，30s后没有连接将会转入SmartConfig
  int timeout = 30000;
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial_debug.println("Wait to connect wifi...");
    display.print(".");
    display.display();
    delay(500);
    timeout = timeout - 300;
    if (timeout <= 0)
    {
      Serial_debug.println("Wifi connect timeout, use smart config to connect...");
      display.print("FAIL, begin SMARTCONFIG");
      display.display();
      SmartConfig();
      return;
    }

    led.flash(2, 125, 125, 0, 0);
  }
  Serial_debug.println("WiFi connected by base config.");
  Serial_debug.print("SSID:");
  Serial_debug.println(WiFi.SSID());
  Serial_debug.print("IP Address:");
  Serial_debug.println(WiFi.localIP().toString());
  display.print("WiFi connected.\nIP:");
  display.print(WiFi.localIP());
  display.display();
}

//WebSocket functions
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    Serial.write(data, len);
  }
}

void onEvent(AsyncWebSocket *server1, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
    case WS_EVT_CONNECT:
      has_active = 1;
      last_active_time = now();
      {
        //CircularBuffer<uint8_t,2000> cached_screen_bytes; 
        uint8_t buf[cached_screen_bytes.size()];
        CircularBuffer<uint8_t,2000>::index_t i;
        for (i = 0; i < cached_screen_bytes.size(); i++)
        {
          buf[i] = cached_screen_bytes[i];
        }
        client->binary(buf, cached_screen_bytes.size());
      }
      
      Serial_debug.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial_debug.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket()
{
  ws.onEvent(onEvent);
  web.addHandler(&ws);
}


//base system setups----------------------------------------------------------------------------
void initDisplay()
{
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial_debug.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
}

void initFS()
{
  //Mount FS
  Serial_debug.println("Mounting FS...");
  if (!SPIFFS.begin())
  {
    Serial_debug.println("Failed to mount file system");
    return;
  }
}
/*
void printDirectory(File dir, int numTabs) 
{
  int colcnt =0;
  while(true) 
  {
    File entry =  dir.openNextFile();
    if (!entry) 
    {
      // no more files
      break;
    }
    if (numTabs > 0) 
    {
      for (uint8_t i=0; i<=numTabs; i++) {
        Serial_debug.print('\t');
      }
    }
    Serial_debug.print(entry.name());
    if (entry.isDirectory()) {
      Serial_debug.println("/");
      printDirectory(entry, numTabs+1);
    } else
    {
      // files have sizes, directories do not
      Serial_debug.print("\t");
      Serial_debug.println(entry.size(), DEC);
    }
    entry.close();
  }
}
*/
void setup()
{
  led.off();
  Serial.setRxBufferSize(1024);
  Serial.begin(115200);
  Serial_debug.begin(115200);

  randomSeed(analogRead(0));
  initDisplay();
  initFS();

  ConnectWifi(); //This may loop forever if wifi is not connected

  initWebSocket();

  // Web Server Root URL
  web.on("/", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/index_all.html", "text/html");
  });

  // Send a GET request to <ESP_IP>/update?state=<inputMessage>
  web.on("/b", HTTP_GET, [] (AsyncWebServerRequest * request)
  {
    String inputMessage;
    int rate = 115200;
    if (request->hasParam("v"))
    {
      inputMessage = request->getParam("v")->value();
      rate = inputMessage.toInt();
      Serial_debug.end();
      Serial.setRxBufferSize(1024);
      Serial.begin(rate);
    }
    request->send(200, "text/plain", "OK");
  });

  web.serveStatic("/", SPIFFS, "/");
  web.begin();

  telnet_server.begin();
  telnet_server.setNoDelay(true);


  Serial_debug.println("Server started");

  led.on();
  last_active_time = now();

  {
     if (!SD.begin(chipSelect)) 
     { // CS is D8 in this example
      Serial_debug.println("Initialising failed!");
      return;
    }
    Serial_debug.println("SD Initialisation completed");
    root = SD.open("/");
    root.rewindDirectory();
    //printDirectory(root, 0); //Display the card contents
    root.close();
    record_file = SD.open("esp_ttl_log.txt", FILE_WRITE);
    if (record_file) record_file_opened = 1;
    //if (record_file) record_file.print("esp_ttl_log");
    //WriteSDFileRecord((uint8_t *)"abc", 3);
    //record_file.close();
    char szTemp[64];
    sprintf(szTemp, "system started.\r\n");
    WriteSDFileRecord((uint8_t* )szTemp, strlen(szTemp));
  }

 
  
}

//loop calls ----------------------------------------------------------------------------

//AcceptTelnetClients
void AcceptTelnetClients()
{
  // Check if there are any new clients ---------
  uint8_t i;
  if (telnet_server.hasClient())
  {
    display.clearDisplay();
    display.setCursor(0, 0);
    for (i = 0; i < MAX_SRV_CLIENTS; i++)
    {
      //find free/disconnected spot
      if (!serverClients[i] || !serverClients[i].connected())
      {
        if (serverClients[i])
        {
          serverClients[i].stop();
        }
        serverClients[i] = telnet_server.available();
        //Serial1.print("New client: "); Serial1.print(i);
        if (serverClients[i].connected())
        {
          display.print("New client: ");
          display.print(serverClients[i].remoteIP());
          display.display();
          has_active = 1;
          last_active_time = now();
        }

        continue;
      }
    }
    // No free/disconnected spot so reject
    WiFiClient serverClient = telnet_server.available();
    serverClient.stop();
  }
}

void CheckTelnetClientData()
{
  unsigned char last_char = 0;
  uint8_t i;
  // check clients for data ------------------------
  for (i = 0; i < MAX_SRV_CLIENTS; i++)
  {
    if (serverClients[i] && serverClients[i].connected())
    {
      if (serverClients[i].available())
      {
        //get data from the telnet client and push it to the UART
        while (serverClients[i].available())
        {
          last_char = serverClients[i].read();
          Serial.write(last_char);
          display.print("<");
          display.display();
        }
      }
    }
  }
}

void CheckSerialData()
{
  uint32_t i;
  // check UART for data --------------------------
  if (Serial.available())
  {
    size_t len = Serial.available();
    uint8_t sbuf[len];
    Serial.readBytes(sbuf, len);
    if (len > 0)
    {
      ws.binaryAll(sbuf, len);
      display.print(">");
      display.display();
      led.flash(2, 20, 20, 0, 0);
      last_active_time = now();
      has_active = 1;
      //push UART data to all connected telnet clients
      for (i = 0; i < MAX_SRV_CLIENTS; i++)
      {
        if (serverClients[i] && serverClients[i].connected()) {
          serverClients[i].write(sbuf, len);
          delay(1);
        }
      }
      WriteSDFileRecord(sbuf, len);
      for (i = 0; i < len; i++)
      {
        cached_screen_bytes.push(sbuf[i]);
      }
    }

    
  }
}

void loop(void)
{
  WiFiWatchDog();
  AcceptTelnetClients();


  if (has_active == 0 && (now() - last_active_time > 60)) //no active after 1min
  {
    if (flashing_ip == 0)
    {
      IPAddress ip = WiFi.localIP();
      led.flash(ip[3], 300, 100, 0, 0); //255*600ms， we need at least 153000ms, 153s to finish
      flashing_ip = 1;
    }
  }
  CheckTelnetClientData();
  CheckSerialData();

  if (display.getCursorY() >= 64)
  {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.display();
  }
}
