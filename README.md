# ![SerrOne](https://raw.githubusercontent.com/claudio-perrotta/SerrOne/master/SerrOne/data/logo.bmp)
Arduino's project about an automated greenhouse; adaptable for AVR Boards, ESP8266 based chips.

## How It Works
- when your ESP starts up and pre-configured, it sets it up in `WIFI_AP_STA` mode and tries to connect to a previously saved Access Point
- in both cases, it serve a configuration portal on `serrone.local`

## Wishlist
- [x] use a Web and DNS server for configuration
- [x] move HTML, CSS, Javascript and other stuff to SPIFFS
- [ ] add ability to configure more parameters than ssid/password
- [ ] ESP32 support or instructions

## Requirements

To use SerrOne with Arduino boards you will need the latest version of the Arduino IDE:

- [Arduino IDE 1.8.6](http://arduino.cc/en/main/software)

### For WiFi using the ESP8266 chip

To use SerrOne with the ESP8266 WiFi chip you will need to install the required module from the Boards Manager of the Arduino IDE. These are the steps to install the ESP8266 package inside the Arduino IDE:

1. Start the Arduino IDE and open the Preferences window
2. Enter `http://arduino.esp8266.com/stable/package_esp8266com_index.json` into the Additional Board Manager URLs field. You can add multiple URLs, separating them with commas.
3. Open the Boards Manager from Tools > Board menu and install the esp8266 package (and after that don't forget to select your ESP8266 board from Tools > Board menu).

### For WiFi using the ESP32 chip

To use SerrOne with the ESP32 WiFi chip you will need to install the board definitions for the Arduino IDE. These are the steps to install support for the ESP32 chip inside the Arduino IDE:

1. Follow the install instructions from [https://github.com/espressif/arduino-esp32](https://github.com/espressif/arduino-esp32)
2. Restart the Arduino IDE
3. Select your ESP32 board from Tools > Board menu.

## Documentation

#### Debug
Debug is enabled by default on Serial. To disable, comment this line:
```cpp
#define ENABLE_DEBUG      true
```

## Releases

#### Version 0.14
+ added control over internet with rest & mqtt api
- know bug with memory heap

#### Version 0.13
+ added rest & mqtt api, for testing
- know bug with PROGMEM macro (disabled some functions)

#### Version 0.12
+ added hash to wi-fi config saved in eeprom
- bug fixes and speed improvements
- ticker.h may not work, used partially now

#### Version 0.11
+ added save/load wi-fi config from eeprom
- Additional fixes & optimisations
- strings now are char array for wi-fi parameters

#### Version 0.10
+ added sub menu and upgrade esisting menu's function
- bug fixes and memory improvements

#### Version 0.9: Initial github release
+ some html, css and js changes
+ added on demand config portal
- bug fixes and speed improvements

#### Version 0.1: First working version
+ LCD menu, with buttons navigations
+ data read on sensors (only DHT here)
+ on/off ~~actuators~~ (only LED here)

Thanks to Ciro Natale for ispiring this project! 👏🏻

## License

This code is released under the GNU GENERAL PUBLIC LICENSE.
