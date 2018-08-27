# SerrOne
Arduino's project about an automated greenhouse; adaptable for AVR Boards, ESP8266 based chips.

## How It Works
- when your ESP starts up, it sets it up in Station mode and tries to connect to a previously saved Access Point

## Wishlist
- [x] move HTML to SPIFFS
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
