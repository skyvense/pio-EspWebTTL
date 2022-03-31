
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

#define STATUS_LED  2
EasyLed led(STATUS_LED, EasyLed::ActiveLevel::Low, EasyLed::State::On);  //Use this for an active-low LED
//EasyLed led(STATUS_LED, EasyLed::ActiveLevel::High, EasyLed::State::On);  //Use this for an active-high LED

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
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
    //Serial.println("Failed to open config file");
    return false;
  }
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, configFile);
  if (error)
  {
    //Serial.println("Failed to read file, using default configuration");
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
    //Serial.println("Load wifi config from spiffs successful.");
    //Serial.print("Loaded ssid: ");
    //Serial.println(_config.SSID);
    //Serial.print("Loaded passwd: ");
    //Serial.println(_config.Passwd);
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
    //Serial.println("Failed to open config file for writing");
    return false;
  }
  if (serializeJson(doc, configFile) == 0)
  {
    //Serial.println("Failed to write to file");
    return false;
  }
  return true;
}


void SmartConfig()
{
  //Serial.println("Use smart config to connect wifi.");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Wifi SmartConfig");
  WiFi.mode(WIFI_STA);
  WiFi.beginSmartConfig();
  while (1)
  {
    //Serial.println("Wait to connect wifi...");
    led.flash(10, 50, 50, 0, 0);
    display.print(".");
    display.display();
    delay(1000);
    if (WiFi.smartConfigDone())
    {
      //Serial.println("WiFi connected by smart config.");
      //Serial.print("SSID:");
      //Serial.println(WiFi.SSID());
      //Serial.print("IP Address:");
      //Serial.println(WiFi.localIP());
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
        //Serial.println("Failed to save config");
      }
      else
      {
        //Serial.println("Config saved");
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
  //Serial.println("Use base config to connect wifi.");
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
    //Serial.println("Wait to connect wifi...");
    display.print(".");
    display.display();
    delay(500);
    timeout = timeout - 300;
    if (timeout <= 0)
    {
      //Serial.println("Wifi connect timeout, use smart config to connect...");
      display.print("FAIL, begin SMARTCONFIG");
      display.display();
      SmartConfig();
      return;
    }

    led.flash(2, 125, 125, 0, 0);
  }
  //Serial.println("WiFi connected by base config.");
  //Serial.print("SSID:");
  //Serial.println(WiFi.SSID());
  //Serial.print("IP Address:");
  //Serial.println(WiFi.localIP());
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
      //Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      //Serial.printf("WebSocket client #%u disconnected\n", client->id());
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
    //Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
}

void initFS()
{
  //Mount FS
  //Serial.println("Mounting FS...");
  if (!SPIFFS.begin())
  {
    //Serial.println("Failed to mount file system");
    return;
  }
}

void setup()
{
  led.off();
  Serial.setRxBufferSize(1024);
  Serial.begin(115200);

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
      //Serial.end();
      Serial.setRxBufferSize(1024);
      Serial.begin(rate);
    }
    request->send(200, "text/plain", "OK");
  });

  web.serveStatic("/", SPIFFS, "/");
  web.begin();

  telnet_server.begin();
  telnet_server.setNoDelay(true);


  //Serial.println("Server started");

  led.on();
  last_active_time = now();
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
  uint8_t i;
  // check UART for data --------------------------
  if (Serial.available())
  {
    size_t len = Serial.available();
    uint8_t sbuf[len];
    Serial.readBytes(sbuf, len);
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
