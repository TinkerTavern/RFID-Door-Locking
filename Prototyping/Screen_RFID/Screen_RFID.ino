// RFID Libraries
#include <SPI.h>
#include <MFRC522.h>
// TFT Screen Libraries
#include <SPI.h>
#include <TFT.h>

// TFT Screen pins, configured to be different
#define cs   8
#define dc   7
#define rst  6  
// RFID Pins, not changed
#define RST_PIN         9           
#define SS_PIN          10       


TFT TFTscreen = TFT(cs, dc, rst);

MFRC522 rfid(SS_PIN, RST_PIN);   // Create MFRC522 instance.

MFRC522::MIFARE_Key key;

// Init array that will store new NUID 
byte nuidPICC[4];

char sensorPrintout[13];

String validIDs[2] = {"59 aa 83 3c", "1d 53 fe 9c"};
void setup() {
  TFTscreen.begin();

  TFTscreen.background(255, 0, 0);

  TFTscreen.stroke(255,255,255);
  TFTscreen.setTextSize(3);
  TFTscreen.text("RFID UID: ",5,40);
  TFTscreen.setTextSize(2);

  Serial.begin(9600); // Initialize serial communications with the PC
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();        // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 
  for (byte i = 0; i < 6; i++) {
      key.keyByte[i] = 0xFF;
  }

  dump_byte_array(key.keyByte, MFRC522::MF_KEY_SIZE);
}

void loop() {
  String sensorVal;
// Look for new cards
  if ( ! rfid.PICC_IsNewCardPresent())
    return;
  
  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  //Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  //Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    //Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  // Store NUID into nuidPICC array
  for (byte i = 0; i < 4; i++) {
    nuidPICC[i] = rfid.uid.uidByte[i];
  }

  TFTscreen.stroke(255,0,0); // Clears the previous text
  TFTscreen.text(sensorPrintout, 10, 70); // Replaces the text with an empty string
//  Serial.println(F("The NUID tag is:"));
//  Serial.print(F("In hex: "));
  sensorVal = dump_byte_array(rfid.uid.uidByte, rfid.uid.size);
  
  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();

  //Serial.println(sensorVal);
  

  sensorVal.toCharArray(sensorPrintout, 13); // Converts the string to a char array for display
  TFTscreen.stroke(255,255,255);
  TFTscreen.text(sensorPrintout, 10, 70);
  //Serial.println(sensorVal);

  
//  tone(5, 1750, 500); // Shows a tag has been read
//  delay(1000);
  tone(5, 3000, 500); // Correct tag
//  delay(1000);
//  tone(5, 100, 500); // Incorrect tag

}

String dump_byte_array(byte *buffer, byte bufferSize) {
    String numbers[4];
    for (int j=0; j < bufferSize; j++) numbers[j] = String(buffer[j], HEX);
//    for (byte i = 0; i < bufferSize; i++) {
//        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
//        Serial.print(buffer[i], HEX);
//    }
    String fullString;
    for (int j=0; j < bufferSize; j++) {
      fullString.concat(numbers[j]);
      fullString.concat(" ");
    }
    return fullString;
}
