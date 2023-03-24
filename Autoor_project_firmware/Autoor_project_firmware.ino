#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <MFRC522.h>
#include "config.h"
#include <Adafruit_Fingerprint.h>

#define LOCK 2
#define BUTTON A0
#define DOORSENSOR 16

constexpr uint8_t RST_PIN =  0;
constexpr uint8_t SS_PIN =  15;

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
SoftwareSerial mySerial(4, 5);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

AdafruitIO_Feed *doorsensor = io.feed("doorsensor_state");
AdafruitIO_Feed *openbutton = io.feed("door_open");

void setup() {
  pinMode(LOCK, OUTPUT);
  pinMode(DOORSENSOR, INPUT);
  pinMode(BUTTON, INPUT);

  digitalWrite(LOCK, LOW);
  
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
  //------------------------(fingerprint sensor)----------------------

  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  finger.getTemplateCount();
//  Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
//  Serial.println("Waiting for valid finger...");

  //-------------------------------------------------------------------

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

  //------------------------------------------------------------
  
  int button_state = analogRead(BUTTON);
  if(button_state > 1000 && read_doorsensor() == 1){
    while(true){
      delay(2000);
      int Time = 0;
      int state = getFingerprintID();
      if(state > 0 || Time == 3) break;
      else Time++;
    }
  }
  
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

  if((code == 3999106697 || 3493325474 || 542010787) && read_doorsensor() == 1){
    digitalWrite(LOCK, HIGH);
    delay(5000);
    digitalWrite(LOCK, LOW);
  }
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence); 

  digitalWrite(LOCK, HIGH);
  delay(5000);
  digitalWrite(LOCK, LOW);

  return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
//int getFingerprintIDez() {
//  uint8_t p = finger.getImage();
//  if (p != FINGERPRINT_OK)  return -1;
//
//  p = finger.image2Tz();
//  if (p != FINGERPRINT_OK)  return -1;
//
//  p = finger.fingerFastSearch();
//  if (p != FINGERPRINT_OK)  return -1;
//  
//  // found a match!
//  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
//  Serial.print(" with confidence of "); Serial.println(finger.confidence);
//  
//  // when match fingerprint, release LOCK.
//
//  digitalWrite(LOCK, HIGH);
//  delay(5000);
//  digitalWrite(LOCK, LOW);
//  return finger.fingerID; 
//}

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
