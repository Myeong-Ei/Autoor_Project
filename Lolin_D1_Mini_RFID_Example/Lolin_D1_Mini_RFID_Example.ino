#include "ESP8266WiFi.h"
#include <SPI.h>
#include <MFRC522.h>
#define LOCK 2   // LOW : ON, HIGH : OFF

constexpr uint8_t RST_PIN =  0;          // Configurable, see typical pin layout above 18
constexpr uint8_t SS_PIN =  15;         // Configurable, see typical pin layout above  16

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

void setup() {
  pinMode(LOCK, OUTPUT);
  digitalWrite(LOCK, LOW);
  Serial.begin(115200);   // Initialize serial communications with the PC
  delay(1000);
  Serial.println("Setup");
  
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
  Serial.println("Setup done");
}

void loop() {
//  Serial.println("Loop...");
  
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    //delay(50);
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    //delay(50);
    return;
  }

  // Show some details of the PICC (that is: the tag/card)
  Serial.print(F("Card UID:"));
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();
  
  // Dump debug info about the card; PICC_HaltA() is automatically called
  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));

  delay(1000);
}

// Helper routine to dump a byte array as hex values to Serial
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
