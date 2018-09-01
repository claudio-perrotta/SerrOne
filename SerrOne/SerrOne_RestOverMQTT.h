/* SerrOne RestOverMQTT */
#pragma once

/* Import required libraries */
//#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <aREST.h>

/* Clients */
WiFiClient espClient;
PubSubClient mqttClient(espClient);

/* Create aREST instance */
aREST rest = aREST(mqttClient);

/* Unique ID to identify the device for cloud.arest.io */
char* device_id = esp_id;

/* Variables to be exposed to the API */
//int temperature;
//int humidity;

/* Custom function accessible by the API */
int deviceControl(String command) {

  /* Get state from command */
  uint8_t devID = (command.substring(command.indexOf('{') + 1, command.indexOf(','))).toInt();
  bool    state = (command.substring(command.indexOf(',') + 1, command.indexOf('}')) == "true") ? true : false;
  Serial.printf("[  REST  ] command: %s, devID: %d, state: %d!\n", command.c_str(), devID, state);

  return aziona(dispositivo.attuatore[devID], state);
}

/* Handles message arrived on subscribed topic(s) */
void callback(char* topic, byte* payload, unsigned int length) {
  rest.handle_callback(mqttClient, topic, payload, length);
}

void restSetup(void) {

  /* Set callback */
  mqttClient.setCallback(callback);

  /* Init variables and expose them to REST API */
  rest.variable(dispositivo.sensore[tempIdx].nome, &dispositivo.sensore[tempIdx].valore);
  rest.variable(dispositivo.sensore[umidIdx].nome, &dispositivo.sensore[umidIdx].valore);
  rest.variable(dispositivo.sensore[terrIdx].nome, &dispositivo.sensore[terrIdx].valore);
  rest.variable(dispositivo.sensore[luceIdx].nome, &dispositivo.sensore[luceIdx].valore);

  /* Function to be exposed */
  rest.function("device", deviceControl);

  /* Give name & ID to the device (ID should be 6 characters long) */
  rest.set_id(device_id);
  rest.set_name("SerrOne");

  /* Connect to WiFi */
  // <------

  /* Set output topic */
  char* out_topic = rest.get_topic();
}

/* End */
