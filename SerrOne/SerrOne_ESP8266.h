/* SerrOne ESP8266 */
#pragma once

/* EEPROM */
#include <EEPROM.h>
int addr = 0;

/* WiFi */
#include <ESP8266WiFi.h>
#include <WiFiClient.h> // E` necessario? Migliora la stabilita` della connessione?

/* Configurazione Wi-Fi */
char*       WFAP_SSID = "SerrOne_AP";
char*       WFAP_PASS = "12345678";
String      WIFI_SSID = "";
String      WIFI_PASS = "";
WiFiMode_t  WIFI_MODE = WIFI_AP;  // Imposta: WIFI_AP | WIFI_STA | WIFI_AP_STA | WIFI_OFF
const char* LOCAL_URL = "serrone.local";
//IPAddress local_IP(192, 168, 4, 22);
//IPAddress gateway(192, 168, 4, 9);
//IPAddress subnet(255, 255, 255, 0);
IPAddress ap_IP(192, 168, 4, 1);
//IPAddress ap_gw(192, 168, 1, 1);
//IPAddress ap_sn(255, 255, 255, 0);

/* Client */
#include <ESP8266HTTPClient.h>
HTTPClient http; // 'http' object is created of class 'HTTPClient'

/* DNSServer */
#include <DNSServer.h>
const byte DNS_PORT = 53;
DNSServer dnsServer;

/* WebServer */
//#define SERVERSECURE 0  // 1 = usa https | 0 = usa http
#ifdef SERVERSECURE
#include <ESP8266WebServerSecure.h>
ESP8266WebServerSecure webServer(443);
static const uint8_t rsakey[]  ICACHE_RODATA_ATTR = {
#include "WebServerSecure_key.h"
};
static const uint8_t x509[] ICACHE_RODATA_ATTR = {
#include "WebServerSecure_x509.h"
};
#else
#include <ESP8266WebServer.h>
ESP8266WebServer webServer(80);
#endif
const char* www_username = "admin";
const char* www_password = "esp8266";

/* FileSystem */
#include <FS.h>

/* Ticker is an object that will call a given function with a certain period. */
#include <Ticker.h>
Ticker tickerPush;
Ticker tickerBlink;

/* Functions */

//SaveJson
String structToJson(S_Dispositivi *s) {
  char adesso_str[80];
  struct tm * timeinfo = localtime(&s->adesso);
  strftime(adesso_str, 80, "%a %d/%m/%Y %T %z", timeinfo);

  char *IS = "  \"", *CS = "\": ", *LS = ",\n", NL = '\n', *TRUE = "true", *FALSE = "false";

  String json = String('{') + NL;
  json += IS + String("Data e ora") + CS + "{\"" + String("T") + CS + '"' + String(adesso_str) + '"' + '}' + LS;
  json += IS + String(s->sensore[tempIdx].nome) + CS + "{\"" + String(s->sensore[tempIdx].simbolo) + CS + String(s->sensore[tempIdx].valore) + '}' + LS;
  json += IS + String(s->sensore[umidIdx].nome) + CS + "{\"" + String(s->sensore[umidIdx].simbolo) + CS + String(s->sensore[umidIdx].valore) + '}' + LS;
  json += IS + String(s->sensore[terrIdx].nome) + CS + "{\"" + String(s->sensore[terrIdx].simbolo) + CS + String(s->sensore[terrIdx].valore) + '}' + LS;
  json += IS + String(s->sensore[luceIdx].nome) + CS + "{\"" + String(s->sensore[luceIdx].simbolo) + CS + String(s->sensore[luceIdx].valore) + '}' + LS;
  json += IS + String(s->attuatore[led_int].nome) + CS + "{\"pin" + String(s->attuatore[led_int].pin) + CS + String(s->attuatore[led_int].stato ? TRUE : FALSE) + '}' + LS;
  json += IS + String(s->attuatore[lampada].nome) + CS + "{\"pin" + String(s->attuatore[lampada].pin) + CS + String(s->attuatore[lampada].stato ? TRUE : FALSE) + '}' + LS;
  json += IS + String(s->attuatore[ventola].nome) + CS + "{\"pin" + String(s->attuatore[ventola].pin) + CS + String(s->attuatore[ventola].stato ? TRUE : FALSE) + '}' + LS;
  json += IS + String(s->attuatore[v_acqua].nome) + CS + "{\"pin" + String(s->attuatore[v_acqua].pin) + CS + String(s->attuatore[v_acqua].stato ? TRUE : FALSE) + '}' + NL;
  json += '}';

  return json;
}

void jsonToFile(String s) {
  bool file_exists = SPIFFS.exists("/data.json");
  File f = SPIFFS.open("/data.json", "a");
#ifdef ENABLE_DEBUG
  Serial.printf("Apertura file in scrittura %s\n", f ? "riuscita!" : "fallita!");
#endif
  if (!f)
    return;
  if (file_exists)
    f.print(",");
  f.print(s.c_str());
  f.close();
}

String recombinePartialJson(char *path) {
  File f = SPIFFS.open(path, "r");
#ifdef ENABLE_DEBUG
  Serial.printf("Apertura file in lettura %s\n", f ? "riuscita!" : "fallita!");
#endif
  if (!f)
    return "Error";
  String json_data = "[";
  while (f.available()) {
    json_data += char(f.read());
    delay(0); // For stability
  }
  f.close();
  json_data += "]";
  return json_data;
}
//EndSaveJson

String httpConnect() {
  String payload;
  if ((WiFi.status() == WL_CONNECTED)) {
    /* allow reuse (if server supports it) */
    http.setReuse(true);

    /* configure traged server and url */
    String clientUrl = "http://claudius.altervista.org/push.php";
    clientUrl += "?token=asdfghjkl";
    clientUrl += "&user=admin";
    //http.begin("https://192.168.1.12/test.html", "7a 9c f4 db 40 d3 62 5a 6e 21 bc 5c cc 66 c8 3e a1 45 59 38"); //HTTPS
    http.begin(clientUrl); //HTTP

    /* config HTTP header */
    http.addHeader("Content-Type", "application/json");

    /* start connection and send HTTP header */
    int httpCode = http.POST(structToJson(&dispositivo));

    /* HTTP header has been send and Server response header has been handled */
    if (httpCode > 0) {
      /* file found at server */
      if (httpCode == HTTP_CODE_OK) {
        payload = http.getString();
      }
    } else {
      /* httpCode will be negative on error */
      payload = String("[HTTP/S] GET... fallita, errore: ") + http.errorToString(httpCode);
    }
    http.end();
  } else {
    /* WiFi error */
    payload = "WiFi Station... non connessa!";
  }
  return payload;
}
void push() {
  httpConnect();
}

void wifiSetup() {
  /* Disconnette dalla rete, se connesso */
  WiFi.softAPdisconnect();
  WiFi.disconnect();

  /* Imposta la modalita` WiFi del modulo ESP */
  WiFi.mode(WIFI_MODE);
  WiFi.hostname(LOCAL_URL);      // DHCP Hostname (useful for finding device for static lease)

  /* Connetti */
  if ( (WIFI_MODE == WIFI_AP_STA) || (WIFI_MODE == WIFI_AP) ) {
    printScreen("Avvio AP...", WFAP_SSID);
    //WiFi.softAPConfig(ap_IP, ap_gw, ap_sn);
    WiFi.softAP(WFAP_SSID, WFAP_PASS);
    printScreen((String("Channel: ") + String(WiFi.channel())).c_str(), String("SoftAPIP: " + WiFi.softAPIP().toString()).c_str());
  }
  if ( (WIFI_MODE == WIFI_AP_STA) || (WIFI_MODE == WIFI_STA) ) {
    printScreen("Connessione a...", WIFI_SSID.c_str());
    //WiFi.config(local_IP, gateway, subnet);  // (DNS not required)
    WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
    /* Aspetta il risultato della connessione */
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      WiFi.mode(WIFI_AP); // da testare!!!
    } else {
      printScreen(String("SSID: " + WiFi.SSID()).c_str(), String("Local IP: " + WiFi.localIP().toString()).c_str());
    }
  }

  /* Connesso! */
  printScreen("Config. WiFi", "terminata!");

#ifdef ENABLE_DEBUG
  Serial.write(12);   // FormFeed
  Serial.println();
  WiFi.printDiag(Serial);
#endif //ENABLE_DEBUG
}

void dnsServerSetup() {
  // modify TTL associated  with the domain name (in seconds)
  // default is 60 seconds
  dnsServer.setTTL(300);
  // set which return code will be used for all other domains (e.g. sending
  // ServerFailure instead of NonExistentDomain will reduce number of queries
  // sent by clients)
  // default is DNSReplyCode::NonExistentDomain
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);

  // start DNS server for a specific domain name
  dnsServer.start(DNS_PORT, LOCAL_URL, ap_IP);
}

void webServerSetup() {
#ifdef SECURESERVER
  webServer.setServerKeyAndCert_P(rsakey, sizeof(rsakey), x509, sizeof(x509));
#endif
  webServer.on("/login", []() {
    if (!webServer.authenticate(www_username, www_password))
      return webServer.requestAuthentication();
    webServer.send(200, "text/plain", "Login OK");
  });

  // handleRoot
  //webServer.serveStatic("/", SPIFFS, "/web/").setDefaultFile("index.html"); // Server with different default file
  webServer.serveStatic("/", SPIFFS, "/web/index.html");
  webServer.serveStatic("/w3.css", SPIFFS, "/web/w3.css");
  webServer.serveStatic("/w3-theme-teal.css", SPIFFS, "/web/w3-theme-teal.css");
  webServer.serveStatic("/fonts.css", SPIFFS, "/web/fonts.css");
  webServer.serveStatic("/logo.svg", SPIFFS, "/web/logo.svg");
  webServer.serveStatic("/arduino.jpg", SPIFFS, "/web/arduino.jpg");
  webServer.serveStatic("/latin-ext.woff2", SPIFFS, "/web/latin-ext.woff2");
  webServer.serveStatic("/latin.woff2", SPIFFS, "/web/latin.woff2");
  webServer.serveStatic("/MaterialIcons-Regular.woff2", SPIFFS, "/web/MaterialIcons-Regular.woff2");

  webServer.on("/push.php", []() {
    webServer.send(200, "text/html", httpConnect());
  });

  webServer.on("/device.php", []() {
    if (webServer.hasArg("device") && webServer.hasArg("ison")) {
      //uint8_t devId = indexOf(webServer.arg("device"));
      bool stato = (webServer.arg("ison") == "true") ? true : false;
      webServer.send(200, "text/plain", aziona(dispositivo.attuatore[led_int], stato) ? "Stato aggiornato" : "Gia` aggiornato");
    } else {
      webServer.send(500, "text/plain", "Comando non valido!");
    }
  });

  webServer.on("/sensors", []() {
    webServer.send(200, "application/json", structToJson(&dispositivo));
  });

  webServer.on("/data.json", []() {
    webServer.send(200, "application/json", recombinePartialJson("/data.json"));
  });

  webServer.on("/deljson", []() {
    SPIFFS.remove("/data.json");
    webServer.send(200, "text/plain", "Cancellato: ./data.json");
  });

  webServer.on("/wifi", []() {
    if (webServer.hasArg("ssid") && webServer.hasArg("pswd")) {
      WIFI_SSID = webServer.arg("ssid");
      WIFI_PASS = webServer.arg("pswd");
      WIFI_MODE = WIFI_AP_STA;
      webServer.send(200, "text/html", "<p>Configurazione WiFi completata</p>");
      setup();
    } else {
      webServer.send(500, "text/html", "<p>Configurazione WiFi fallita!!!</p>");
    }
  });

  // handleNotFound
  webServer.onNotFound([]() {
    String message = "Oh no, questo link non esiste!\nOra il mondo finira`... per colpa tua!\n\n";
    message += "URI: ";
    message += webServer.uri();
    webServer.send(404, "text/plain", message);
  });

  webServer.begin();
}

/* End */

