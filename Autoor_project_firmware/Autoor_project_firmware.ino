#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <MFRC522.h>
#include "config.h"

#define LOCK 2
#define DOORSENSOR 16

constexpr uint8_t RST_PIN =  0;
constexpr uint8_t SS_PIN =  15;

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

AdafruitIO_Feed *doorsensor = io.feed("doorsensor_state");
AdafruitIO_Feed *openbutton = io.feed("door_open");

void setup() {
  pinMode(LOCK, OUTPUT);
  pinMode(DOORSENSOR, INPUT);
  
  Serial.begin(115200);
  Serial.println("Setup");
  while(! Serial);

  //----------------------(RFID)--------------------------------------

  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
  Serial.println("Setup done");

  //------------------------------------------------------------------
  

  Serial.print("Connecting to Adafruit IO");

  io.connect();

  openbutton->onMessage(control_lock);

  while(io.mqttStatus() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  openbutton->get();

  Serial.println();
  Serial.println(io.statusText());

}

void loop() {
  io.run();
  doorsensor->save(read_doorsensor());
  
  //---------------------------------------------------------------
  
  if (mfrc522.PICC_IsNewCardPresent()) {
    if (mfrc522.PICC_ReadCardSerial()){
      // Show some details of the PICC (that is: the tag/card)
      Serial.print(F("Card UID:"));
      dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
      Serial.println();
    }
  }
  
  //------------------------------------------------------------
  
  delay(3000);
}

// Helper routine to dump a byte array as hex values to Serial(RFID)
void dump_byte_array(byte *buffer, byte bufferSize) {
  uint64_t code = 0;
  for (byte i = 0; i < bufferSize; i++) {
    //Serial.print(buffer[i], HEX);
    code |= ((uint64_t) buffer[i] << ((bufferSize - i - 1) * 8));
  }
  Serial.println(code);

  if(code == 3999106697 || 3493325474 || 542010787){
    digitalWrite(LOCK, HIGH);
    delay(5000);
    digitalWrite(LOCK, LOW);
  }
}

void control_lock(AdafruitIO_Data *data) {

  Serial.print("Button detected : ");
  Serial.println(data->value());
  int state = data->toInt();
  
//  if(state == 1)digitalWrite(LOCK, LOW);
//  else digitalWrite(LOCK, HIGH);

  if(state == 1 && read_doorsensor() == 1){
    digitalWrite(LOCK, HIGH);
    delay(5000);
    digitalWrite(LOCK, LOW);
  }
}

int read_doorsensor() {
  int value = digitalRead(DOORSENSOR);
  Serial.print("door sensor value : ");
  Serial.println(value);
  return value;
}
