# Esp266 WebTTL
A very simple &amp; fast WebTTL based on ESP8266 Wifi Module, coding by PlatformIO/Arduino

The program running on an ESP8266 board allows you to access your device TTL remotely from any device that has a browser.

The project is fully functioning and the speed of controlling TTL is the same as you connect the cable via an USB-TTL device.

## Build steps
1. Install PlatformIO(https://www.youtube.com/watch?v=JmvMvIphMnY)
2. Install Adafruit BusIO
3. Build the project and upload to an ESP8266 board
4. Modify SSID information in data/config.json.
5. Build SPIFFS image with "Build Filesystem Image" and upload to the board.

## Wire Connecting 
6. Connect SSD3306 OLED I2C correctly
7. Reset the board and the board will startup connecting your SSID.
8. Another option: remove config.json, the board will startup in Wifi SmartConfig mode, use ESP8266 smartconfig Android app to send wifi information.
9. After the board is connected with wifi and got correct IP, like this:
![IP](/pic/desktop.jpg)

10. Use a browser on your PC, enter the IP: http://IP and input your TTL wire baud rate, like:
![Browser](/pic/browser.png)

11. Then you should see the screen like this:
![RunningPic](/pic/running.png)


## Telnet/23 support
You can also connect to IP:23 with a telnet command from any internet/LAN device.

## Input & Output contents sync
All contents are sync displayed if multiple clients is connected, no matter telnet or browser

## Works without an OLED display
If you don't have an OLED display, you can got your IP ADDRESS from your router DHCP offered page. OR HERE: you can count the LED blink times after the Wifi Connected for 1 minute(TTL and network should be IDLE). Like, if your IP address is 192.168.2.15, the default LED on the board will BLINK 15 times!

