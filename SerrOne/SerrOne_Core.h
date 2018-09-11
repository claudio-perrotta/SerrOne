/* SerrOne_Core.h */
#pragma once

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
    {"LED integrato", LED_PIN, LOW},
    {"Lampada", LIGHT_PIN, LOW},
    {"Ventola", FAN_PIN, LOW},
    {"Valvola acqua", WATER_VALVE, LOW}
  };
} dispositivo;

enum sensorsIdx   {tempIdx = 0, umidIdx = 1, terrIdx = 2, luceIdx = 3};   // Indici sensori
enum actuatorsIdx {led_int = 0, lampada = 1, ventola = 2, v_acqua = 3};   // Indici attuatori

/* Variabili globali disponibili ovunque nel programma */
unsigned long       tempo_corrente;   // Current time
unsigned long       tempo_iniziale;   // Definizione della variabile da inizializzare successivamente all'avvio
const unsigned long periodo = 2500;   // Il valore e` il numero dei millisecondi minimo per l'aggiornamento

bool aggiornaSensori(void) {
  /* Ottieni il "tempo" corrente (attualmente il numero di millisecondi dall'avvio del programma) */
  tempo_corrente = millis();
  /* Controlla quando il periodo e` superato - Test whether the period has elapsed */
  if (tempo_corrente - tempo_iniziale >= periodo) {
    dispositivo.adesso = time(nullptr);
    dispositivo.sensore[tempIdx].valore = dht.readTemperature();
    dispositivo.sensore[umidIdx].valore = dht.readHumidity();
    dispositivo.sensore[terrIdx].valore = constrain(map(analogRead(SM_PIN), 0, 1023, 0, 100), 0, 100);
    dispositivo.sensore[luceIdx].valore = 127;
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
      Serial.printf("[ ACTION ] Dispositivo sul pin #%d (%s) stato: %s\n", a.pin, a.nome, a.stato ? "ON!" : "OFF");
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
      Serial.printf("[ ACTION ] Dispositivo sul pin #%d (%s) stato: %s\n", a.pin, a.nome, a.stato ? "ON!" : "OFF");
#endif //ndef AVR
#endif //ENABLE_DEBUG
      return true;
    }
  }
  return false; // Attuatore gia` aggiornato
}

/* Metodo per controllare gli attuatori */
void controllaAutomatizzazione(void) {
  aziona(dispositivo.attuatore[lampada],  dispositivo.sensore[umidIdx].valore < 55);
  aziona(dispositivo.attuatore[ventola], (dispositivo.sensore[tempIdx].valore > 30) || (dispositivo.sensore[umidIdx].valore > 85) );
  aziona(dispositivo.attuatore[v_acqua],  dispositivo.sensore[terrIdx].valore < 40);
}

/* Routine per il polling */
void polling(void); // Only interface

/* End */
