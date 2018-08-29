/*
   SerrOne

   Gestisce autonomamente una piccola serra.
   Utilizza un menu e permette di navigare al suo interno.

   Created 08-Jun-2018
   by Claudio Perrotta

   This code is in the public domain.
*/
//#include <Arduino.h>
#include "SerrOne_Version.h"

/* Configurazione debug */
#define ENABLE_DEBUG      true    // true = attiva debug | false = disattiva debug
#define SERIAL_BAUDRATE   115200  // Velocita` porta seriale

/* Configurazione PIN sensori ed attuatori */
#define DHT_PIN     15            // Il pin digitale al quale e` connesso il DHT
#define DHT_TYPE    DHT11         // Tipo di sensore DHT
#define SM_PIN      A0            // PIN sensore umidita` suolo (Soil Moisture Sensor's PIN)
#define LIGHT_PIN   LED_BUILTIN   // PIN della luce
#define FAN_PIN     0             // PIN della ventola
#define WATER_VALVE 0             // PIN della valvola acqua

/* Configurazione PIN tasti */
#define BUTTON_A    15            // Select
#define BUTTON_B    12            // OK

/* Include la libreria Wire per il bus I2C */
#include <Wire.h>                 // Libreria Wire per il bus I2C
//#define SCL_PIN     D1            // Il pin digitale della linea SCL (Shared clock signal)
//#define SDA_PIN     D2            // Il pin digitale della linea SDA (Shared data line)

/* Inclusione libreria sensore */
#include <DHT.h>                  // Adafruit's library, per gestire il sensore DHT
DHT dht(DHT_PIN, DHT_TYPE);       // Inizializza dht

/* Inclusione libreria orario */
#include <time.h>
int timezone = 2;                 // Fuso orario, in ore
int dst = 0;                      // Ora legale, in ore

/* Variabili globali disponibili ovunque nel programma */
unsigned long       tempo_corrente;   // Current time
unsigned long       tempo_iniziale;   // Definizione della variabile da inizializzare successivamente all'avvio
const unsigned long periodo = 2500;   // Il valore e` il numero dei millisecondi minimo per l'aggiornamento

/* Struttura dati per gli attuatori (Actuators) */
struct S_Attuatore {
  char      *nome;
  uint8_t   pin;
  bool      stato;
};

/* Struttura dati per i sensori (Sensors) */
struct S_Sensore {
  char      *nome;
  uint8_t   valore;
  char      *simbolo;
};

/* Struttura dati per i dispositivi */
struct S_Dispositivi {
  time_t adesso;                  // Data ed ora (tempo)
  /* Definisce ed inizializza i sensori */
  S_Sensore sensore[4] = {
    {"Temperatura", 0, "C"},      // Temperature sensing
    {"Umidita`", 0, "%"},         // Humidity sensing
    {"Umidita` terra", 0, "%"},   // Soil moisture sensing
    {"Luminosita`", 0, "cd"}      // Ambient light sensing
  };
  /* Definisce ed inizializza gli attuatori */
  S_Attuatore attuatore[4] = {
    {"LED integrato", LED_BUILTIN, LOW},
    {"Lampada", LIGHT_PIN, LOW},
    {"Ventola", FAN_PIN, LOW},
    {"Valvola acqua", WATER_VALVE, LOW}
  };
} dispositivo;

enum sensorsIdx   {tempIdx = 0, umidIdx = 1, terrIdx = 2, luceIdx = 3};   // Indici sensori
enum actuatorsIdx {led_int = 0, lampada = 1, ventola = 2, v_acqua = 3};   // Indici attuatori

bool aggiornaSensori() {
  /* Ottieni il "tempo" corrente (attualmente il numero di millisecondi dall'avvio del programma) */
  tempo_corrente = millis();
  /* Controlla quando il periodo e` superato - Test whether the period has elapsed */
  if (tempo_corrente - tempo_iniziale >= periodo) {
    dispositivo.adesso = time(nullptr);
    dispositivo.sensore[tempIdx].valore = dht.readTemperature();
    dispositivo.sensore[umidIdx].valore = dht.readHumidity();
    dispositivo.sensore[terrIdx].valore = constrain(map(analogRead(SM_PIN), 0, 1023, 0, 100), 0, 100);
    /* IMPORTANT to save the start time of the current sensors state */
    tempo_iniziale = tempo_corrente;
    //jsonToFile(structToJson(&dispositivo));
    return true;
  } else return false;
}

/* Funzione condizionale per azionare gli attuatori */
bool aziona(S_Attuatore &a, bool condizione = true) {
  pinMode(a.pin, OUTPUT);
  if (condizione) {
    if (a.stato != HIGH) {
      digitalWrite(a.pin, HIGH);
      a.stato = HIGH;
#ifdef ENABLE_DEBUG
#ifndef AVR
      Serial.printf("[AZIONA] Dispositivo sul pin #%d (%s) stato: %s\n", a.pin, a.nome, a.stato ? "ON!" : "OFF");
#endif //ndef AVR
#endif //ENABLE_DEBUG
      return true;
    }
  } else {
    if (a.stato != LOW) {
      digitalWrite(a.pin, LOW);
      a.stato = LOW;
#ifdef ENABLE_DEBUG
#ifndef AVR
      Serial.printf("[AZIONA] Dispositivo sul pin #%d (%s) stato: %s\n", a.pin, a.nome, a.stato ? "ON!" : "OFF");
#endif //ndef AVR
#endif //ENABLE_DEBUG
      return true;
    }
  }
  return false; // Attuatore gia` aggiornato
}

/* Metodo per controllare gli attuatori */
void controllaAutomatizzazione() {
  aziona(dispositivo.attuatore[lampada],  dispositivo.sensore[umidIdx].valore < 55);
  aziona(dispositivo.attuatore[ventola], (dispositivo.sensore[tempIdx].valore > 30) || (dispositivo.sensore[umidIdx].valore > 85) );
  aziona(dispositivo.attuatore[v_acqua],  dispositivo.sensore[terrIdx].valore < 40);
}

/* Inclusione librerie per il menu e lo schermo */
#include "SerrOne_Screen.h"

/* Inclusione librerie esclusive per ESP */
#if defined(AVR)
//#include <Arduino.h> // <- placeholder

#elif defined(ESP8266) || defined(ESP32)
#include "SerrOne_ESP.h"

#else
#error Platform not supported

#endif //Platforms

/* Routine per il polling */
void polling(void) {
  aggiornaSensori();
  controllaAutomatizzazione();
#ifdef ESP8266
  dnsServer.processNextRequest();
  webServer.handleClient();
  yield(); // Per la compatibilita` con i servizi in background dell'ESP
#ifdef ENABLE_DEBUG
  static unsigned long last = millis();
  if (millis() - last > 5000) {
    last = millis();
    Serial.printf("[POLLING] Free heap: %d bytes\n", ESP.getFreeHeap());
  }
#endif //ENABLE_DEBUG
#endif //ESP8266
}

/* Imposta lo stato del PIN */
void BalternaStatoPIN(int pin, bool& stato) {
  pinMode(pin, OUTPUT);
  stato = !stato; // Inverte lo stato
  digitalWrite(pin, stato);
  printScreen((String("DISP. SUL PIN ") + pin).c_str(), stato ? "Dispositivo ON! " : "Dispositivo OFF ");
}

/* Funzionalita` 1 */
void attuatoreLuce() {
  String ln_1 = String(dispositivo.attuatore[led_int].nome);
  String ln_2 = String(dispositivo.attuatore[led_int].stato ? "Dispositivo ON!" : "Dispositivo OFF");
  printScreen(ln_1.c_str(), ln_2.c_str());
  bool stato_precedente = dispositivo.attuatore[led_int].stato;
  while (true != pressioneTasto(BUTTON_A)) {
    if (true == pressioneTasto(BUTTON_B))
      aziona(dispositivo.attuatore[led_int], !dispositivo.attuatore[led_int].stato);
    if (stato_precedente != dispositivo.attuatore[led_int].stato) {
      String ln_1 = String(dispositivo.attuatore[led_int].nome);
      String ln_2 = String(dispositivo.attuatore[led_int].stato ? "Dispositivo ON!" : "Dispositivo OFF");
      printScreen(ln_1.c_str(), ln_2.c_str());
      stato_precedente = dispositivo.attuatore[led_int].stato;
    }
    polling();
  }
}

/* Funzionalita` 2 */
void sensoreDHT() {
  printScreen("", msg_attendere, false);
  do {
    if (aggiornaSensori() == true) {
      String str_t = String("Temperatura  ") + dispositivo.sensore[tempIdx].valore + String("C");
      String str_h = String("Umidita`     ") + dispositivo.sensore[umidIdx].valore + String("%");
      printScreen(str_t.c_str(), str_h.c_str());
    }
    polling();
  } while (true != pressioneTasto(BUTTON_A));
}

/* Funzionalita` 3 */
void sensoreTerreno() {
  printScreen("", msg_attendere, false);
  do {
    if (aggiornaSensori() == true) {
      String str_ht = String("Umid. terra  ") + dispositivo.sensore[terrIdx].valore + String("%");
      printScreen("SENSORE TERRENO ", str_ht.c_str());
    }
    polling();
  } while (true != pressioneTasto(BUTTON_A));
}

/* Funzionalita` 4 */
void sensoreOhm() {
  printScreen("", msg_attendere, false);
  int Vin = 5; // Volt in uscita dal Pin 5V di Arduino
  float R1 = 1000; // impostare il valore della resistenza nota in Ohm
  float R2 = 0;
  do {

    if (aggiornaSensori() == true) {
      R2 = R1 * ( ( Vin / ( ( analogRead(A0) * Vin ) / 1024.0 ) ) - 1 );
      String str_ohm = String("R2: ") + R2 + String(" Ohm");
      printScreen("OHM METER R1=1kO", str_ohm.c_str());
    }
    polling();
  } while (true != pressioneTasto(BUTTON_A));
}

/* Funzionalita` 5 */
void ScreenSaver() {
  ssd1306_drawBitmap(0, 0, 128, 32, logo);
#ifndef AVR
  tickerBlink.attach(1, invertMode);
#endif //ndef AVR
  do {
    polling();
  } while (true != pressioneTasto(BUTTON_A));
#ifndef AVR
  tickerBlink.detach();
#endif //ndef AVR
  invertModeState = false;
  invertMode();
}

/* Menu comandi */
SMenu<4> menu_comandi[] = {
  /* Titolo del menu */
  "     COMANDI    ",
  /* Voci e relative funzioni del menu */
  {
    { "1. LED On/Off   ", []() {
        attuatoreLuce();
      }
    },
    { "2. Sensore DHT  ", []() {
        sensoreDHT();
      }
    },
    { "3. Sensore terr.", []() {
        sensoreTerreno();
      }
    },
    { "4. Misurat. Ohm ", []() {
        sensoreOhm();
      }
    }
  }
};

/* Menu sistema */
SMenu<2> menu_sistema[] = {
  /* Titolo del menu */
  "     SISTEMA    ",
  /* Voci e relative funzioni del menu */
  {
    { "1. Screen Saver ", []() {
        ScreenSaver();
      }
    },
    { "2. Versione FW  ", []() {
        printScreen("", Version::toString(), false); delay(2000);
      }
    }
  }
};

/* Menu princiale */
SMenu<3> menu_principale[] = {
  /* Titolo del menu */
  "      MENU      ",
  /* Voci e relative funzioni del menu */
  {
    { "Premi il pul. A ", []() {
        printScreen("", "Premuto pul. B! ", false); delay(2000);
      }
    },
    { "1. Comandi      ", []() {
        runMenu(menu_comandi);
      }
    },
    { "2. Sistema      ", []() {
        runMenu(menu_sistema);
      }
    }
  }
};

void setup() {
#ifdef ENABLE_DEBUG
  /* Inizializza Serial Monitor */
  Serial.begin(SERIAL_BAUDRATE);
#ifndef AVR
  Serial.printf("\n[DEBUG] SerrOne - ver. %s\n[DEBUG] Compilation began %s at %s with C++%ld\n", Version::toString(), __DATE__, __TIME__, __cplusplus);
#endif //ndef AVR
#endif //ENABLE_DEBUG
  /* Initial start time */
  tempo_iniziale = millis();
  /* Inizializza il pin digitale LED_BUILTIN come un output ed accendi il LED */
  aziona(dispositivo.attuatore[led_int], true); // Passa da LOW a HIGH
  /* Inizializza schermo */
  screenSetup();
  /* Splash screen */
  splashScreen();
#ifdef ESP8266
  /* Inizializza l'ogetto del File System (SPIFFS) */
  SPIFFS.begin();
  /* Configura il WiFi ed instanzia una connessione */
  wifiSetup();
  /* Configura l'orario */
  configTime(timezone * 3600, dst * 3600, "pool.ntp.org", "time.nist.gov");
  if (WiFi.getMode() != WIFI_AP) {
    for (uint8_t i = 1; i <= 10; i++) {
      printScreen("Config. ora...", String(i).c_str());
      if (time(nullptr) > 38880) break;
      delay(1000);
    }
  }
  /* Configura il Server DNS */
  dnsServerSetup();
  /* Configura il Web Server */
  webServerSetup();
  /* Scheduler per l'invio dei dati al server */
  tickerPush.attach(60, push);
#endif //ESP8266
  /* Pulisci LCD e spegni il LED */
#ifdef USE_LCD
  lcd.clear();
  lcd.noCursor();
  lcd.noBlink();
#endif //USE_LCD
  aziona(dispositivo.attuatore[led_int], false); // Passa da HIGH a LOW
}

void loop() {
  polling();
  runMenu(menu_principale);
}

/*
   Remember that there is a lot of code that needs to run on the chip besides the sketch when WiFi is connected.
   WiFi and TCP/IP libraries get a chance to handle any pending events each time the loop() function completes,
   OR when delay is called. If you have a loop somewhere in your sketch that takes a lot of time (>50ms) without
   calling delay, you might consider adding a call to delay function to keep the WiFi stack running smoothly.

   There is also a yield() function which is equivalent to delay(0). The delayMicroseconds function, on the other
   hand, does not yield to other tasks, so using it for delays more than 20 milliseconds is not recommended.
*/
