#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "config.h"

#define LOCK 2
#define DOORSENSOR 16

AdafruitIO_Feed *relay = io.feed("relay");
AdafruitIO_Feed *doorsensor = io.feed("door_sensor");

void setup() {
  pinMode(LOCK, OUTPUT);
  pinMode(DOORSENSOR, INPUT);
  Serial.begin(115200);

  while(! Serial);

  Serial.print("Connecting to Adafruit IO");

  io.connect();

  relay->onMessage(controll_lock);

  while(io.mqttStatus() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  relay->get();

  Serial.println();
  Serial.println(io.statusText());

}

void loop() {
  io.run();
  read_doorsensor();
  delay(2000);
}

void controll_lock(AdafruitIO_Data *data) {

  Serial.print("received <- ");
  Serial.println(data->value());
  int state = data->toInt();
  
  if(state == 1)digitalWrite(LOCK, LOW);
  else digitalWrite(LOCK, HIGH);

}

void read_doorsensor() {
  int value = digitalRead(DOORSENSOR);

  Serial.print("door sensor value : ");
  Serial.println(value);
  doorsensor->save(value);
}
