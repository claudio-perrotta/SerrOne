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

## Documentation

#### Debug
Debug is enabled by default on Serial. To disable, comment this line:
```cpp
#define ENABLE_DEBUG      true
```

## Releases
#### 0.9
* initial release
- bug fixes and speed improvements
+ some html, css and js changes
+ added on demand config portal
