/* SerrOne Screen */
#pragma once

//#define USE_LCD 0   // 1 = usa LCD | 0 = usa OLED
#ifdef USE_LCD
/* Include la libreria ed imposta l'indirizzo I2C del LCD */
#include <LiquidCrystal_I2C.h>    // NewLiquidCrystal library, per gestire LCD
#define LCD_ADDR    0x27          // Indirizzo I2C del LCD (attraverso IO expansion module)
uint8_t LCD_W       16            // Larghezza LCD in caratteri
uint8_t LCD_H       2             // Altezza LCD in caratteri
//const byte Address=0x27, EN=2, RW=1, RS=0, D4=4, D5=5, D6=6, D7=7, BackLight=3, BL_Polarity=POSITIVE;
LiquidCrystal_I2C lcd(LCD_ADDR, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
#else
/* Include la libreria per l'OLED */
#include "ssd1306.h"
//WIFI_Kit_8's OLED connection:
//SDA -- GPIO4  -- D2
//SCL -- GPIO5  -- D1
//RST -- GPIO16 -- D0
#define RST_OLED 16
#include "SerrOne_Resources.h"
#endif //USE_LCD

/* Inverte i colori dello schermo */
bool invertModeState = false;
void invertMode() {
  invertModeState = !invertModeState;
  if (invertModeState) {
    ssd1306_normalMode();
  } else {
    ssd1306_invertMode();
  }
}

/* Messaggi predefiniti */
#define MAX_STRING  16+1       // Larghezza LCD + terminatore '\0' (Attualmente, 16+1 caratteri)
char msg_attendere[MAX_STRING] = "Attendere...    ";

/* Funzione per stampare a video */
void printScreen(const char *line0, const char *line1, bool need_clear = true) {
#ifdef USE_LCD
  if (need_clear)
    lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line0);
  lcd.setCursor(0, 1);
  lcd.print(line1);
#else
  if (need_clear)
    ssd1306_fillScreen(0x00);
  ssd1306_printFixed (0,  0, line0, STYLE_BOLD);
  ssd1306_printFixed (0, 16, line1, STYLE_NORMAL);
#endif //USE_LCD
#ifdef ENABLE_DEBUG
  if (need_clear)
    Serial.write(12);   // FormFeed
  Serial.println("\n[ SCREEN ]");
  Serial.println("+----------------+");
  Serial.print("|"); Serial.print(line0); Serial.println("|");
  Serial.print("|"); Serial.print(line1); Serial.println("|");
  Serial.println("+----------------+");
#endif //ENABLE_DEBUG
}

/* Inizializza schermo */
void screenSetup() {
  //Wire.begin(SDA_PIN, SCL_PIN);
#ifdef USE_LCD
  lcd.begin(LCD_W, LCD_H);
  lcd.cursor();
  lcd.blink();
#else
  pinMode(RST_OLED, OUTPUT);
  digitalWrite(RST_OLED, LOW); // turn D2 low to reset OLED
  delay(50);
  digitalWrite(RST_OLED, HIGH); // while OLED is running, must set D2 in high
  ssd1306_128x32_i2c_init();
  ssd1306_setFixedFont(ssd1306xled_font6x8);
#endif //USE_LCD
}

/* Splash screen e avanzamento barra in 2 secondi */
void splashScreen() {
  printScreen("    SerrOne!    ", "................");
#ifdef USE_LCD
  lcd.setCursor(0, 1);
  for (uint8_t i = 0; i < LCD_W; i++) {
    lcd.print('_');
    delay(2000 / LCD_W);
  }
#else
  //ssd1306_drawBitmap(0, 0, 128, 32, logo);
  for (uint8_t i = 0; i < 16; i++) {
    ssd1306_printFixed (i * 8, 32, "_", STYLE_NORMAL);
    delay(2000 / 16);
  }
#endif //USE_LCD
}

/* Controlla quale lettera viene immessa da terminale */
#ifdef ENABLE_DEBUG
bool serialEvent(char c = '\n') {
  // Associa il tasto
  if (c == BUTTON_A) c = 'a';
  else if (c == BUTTON_B) c = 'b';
  // Controlla l'input
  if (Serial.available()) {
    char inChar = char(Serial.read());
    if (inChar == c)
      return true;
  }
  return false;
}
#endif //ENABLE_DEBUG

/* Controlla la pressione di un tasto */
bool pressioneTasto(int pin) {
#ifdef ENABLE_DEBUG
  return serialEvent(pin); // Aggira il controllo dei tasti fisici [ButtonOverride]
#endif //ENABLE_DEBUG
  pinMode(pin, INPUT);
  bool stato = digitalRead(pin); // Controlla se il tasto e` stato premuto [Trigger]
  if (stato == LOW)
    return false;
  else
    while (stato == HIGH) // Aspetta il rilascio del tasto (anti-rimbalzo) [Debouncing]
      delay(0); // Per la compatibilita` con i servizi in background dell'ESP
  stato = digitalRead(pin);
  return true; // Restituisce il risultaato al rilascio
}

/* Definizione dell'handler della funzione per il menu */
#ifdef ESP8266
#include <functional>
typedef std::function<void(void)> THandlerFunction;
#else
typedef void (*THandlerFunction)(void); // = NULL; direttamente nella funzione?
#endif

/* Definizione della struttura dati per il menu */
typedef struct SMenuVoice {
  char label[MAX_STRING];
  THandlerFunction fn;
} SMenuVoice;

template <uint8_t N>
struct SMenu {
  char *title;
  SMenuVoice element[N];
};

/* Variabili globali di stato */
bool    menu_callback_exit = false; // Inizializza menu_callback_exit
uint8_t menu_item_id = 0;           // Inizializza variabile menu corrente

/* Funzione per eseguire il menu */
template <uint8_t N>
void runMenu(SMenu<N> *menu) {
  size_t size_vList = sizeof(menu->element);    // Size of list
  size_t size_voice = sizeof(*menu->element);   // Size of 1 element
  const uint8_t menu_items = (size_vList / size_voice) - 1; // Calcolo del numero di voci del menu dalla lista, meno uno
  THandlerFunction _callback(NULL); // Handler per il callback
  uint8_t prev_menu_item_id = 0;    // Dichiara ed inizializza la variabile
  bool    menu_back = false;        // Inizializza menu back
  menu_callback_exit = false;       // Reimposta lo stato
  /* Il ciclo DO-WHILE, avvia il menu */
  do {
    if (menu_item_id == menu_items + 1) {
      printScreen(menu->title, "<- Back         ", false);
    } else {
      printScreen(menu->title, menu->element[menu_item_id].label, false);
    }
    while (true != pressioneTasto(BUTTON_A)) {
      if (true == pressioneTasto(BUTTON_B)) {
        if (menu_item_id == menu_items + 1) {
          menu_back = true;
          menu_callback_exit = true; // In questo caso, per ristampare il menu
        } else {
          _callback = menu->element[menu_item_id].fn; // Assegna il callback
          if (_callback) {
            prev_menu_item_id = menu_item_id; // Registra lo stato del menu attuale
            menu_item_id = 0; // Reimposta lo stato del menu alla prima voce di default
            _callback(); // Richiama il metodo (callback)
            menu_callback_exit = true; // Rileva l'uscita dal callback
          } else {
            printScreen("ERRORE", "Non implementato"); delay(2000); menu_callback_exit = true;
          }
        }
      }
      polling();
      delay(1); // Per stabilita`
      if (menu_callback_exit == true) break; // Condizione d'uscita, esce dal while
    } // Fine WHILE BUTTON_A
    if (menu_callback_exit == true) break; // Condizione d'uscita, esce dal do-while saltando l'incremento
    menu_item_id++;
    if (menu_item_id > menu_items + 1) {
      menu_item_id = 0;
    }
  } while (!menu_back); // Ripete il ciclo DO-WHILE, altrimenti!
  menu_item_id = prev_menu_item_id; // Uscendo dal menu, ritorna alla stessa voce da dove invocato il sotto-menu
}

/* End */
