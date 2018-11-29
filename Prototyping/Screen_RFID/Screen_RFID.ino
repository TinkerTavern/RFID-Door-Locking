

// RFID Libraries
#include <SPI.h>
#include <MFRC522.h>
// button interrupt library
int answerButton1 = 2;
int answerButton2 = 3;
// TFT Screen Libraries
#include <SPI.h>
#include <TFT.h>
//bluetooth libraries
#include <SoftwareSerial.h>
//initializing bluetooth connection
SoftwareSerial mySerial(4, 5); 
// TFT Screen pins, configured to be different
#define cs   8
#define dc   7
#define rst  6  
// RFID Pins, not changed
#define RST_PIN         9           
#define SS_PIN          10       

String RFIDValid = "f";
bool questionRight = false;
bool questionAnswered = false;

TFT TFTscreen = TFT(cs, dc, rst);

MFRC522 rfid(SS_PIN, RST_PIN);   // Create MFRC522 instance.

MFRC522::MIFARE_Key key;

// Init array that will store new NUID 
byte nuidPICC[4];

char sensorPrintout[13];
bool faceDetected = false;

void setup() {
  TFTscreen.begin();
  
  mySerial.begin(38400);
  Serial.begin(9600); // Initialize serial communications with the PC
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();        // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 
  for (byte i = 0; i < 6; i++) {
      key.keyByte[i] = 0xFF;
  }
  attachInterrupt(0, answer1, FALLING); //specifying which interrupt pins correspond to which pin, and specifying the ISRs
  attachInterrupt(1, answer2, FALLING);

  pinMode(answerButton1, OUTPUT); //set player buttons as inputs
  pinMode(answerButton2, OUTPUT);

  TFTscreen.background(255, 0, 0);
  TFTscreen.stroke(255,255,255);
  TFTscreen.text("Please scan RFID card", 5, 40); // Replaces the text with an empty string
}

void detectFace() {
  
}


void loop() {
  if (!faceDetected) { 
    String newString;
    if (Serial.available()) {
      while (Serial.available()) {
        delay(10);  //small delay to allow input buffer to fill
        char c = Serial.read();  //gets one byte from serial buffer
        newString += c;
      } //makes the string readString
    
    if (newString.length() > 0) {
      if (newString == "True") faceDetected = true;
    }
  }
  }
  else {
  if (RFIDValid == "f"){
  String sensorVal;
// Look for new cards
  if (! rfid.PICC_IsNewCardPresent())
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

 // TFTscreen.stroke(255,0,0); // Clears the previous text
 // TFTscreen.text(sensorPrintout, 10, 70); // Replaces the text with an empty string

  sensorVal = byteToStringArray(rfid.uid.uidByte, rfid.uid.size);


  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();

  //Serial.println(sensorVal);
  
  sensorVal.toCharArray(sensorPrintout, 13); // Converts the string to a char array for display
  mySerial.write(sensorPrintout); // Sends the RFID tag for verification
  delay(200); // Delay to ensure the slave's response is finished before checking
  RFIDValid = recieveCommand();

  if (RFIDValid == "t"){
    TFTscreen.background(255, 0, 0);
    TFTscreen.stroke(255,255,255);
    TFTscreen.text("Access Granted", 5, 40); // Replaces the text with an empty string
    //mySerial.write("Dont Kill");
    tone(A0, 3000, 500); // Correct tag
    delay(1000);
    TFTscreen.background(255, 0, 0);
    TFTscreen.stroke(255,255,255); // Clears the previous text
    TFTscreen.text("Whats The Password?", 5, 20); // Replaces the text with an empty string
    TFTscreen.text("snow", 5, 40); // Replaces the text with an empty string
    TFTscreen.text("angels", 5, 60); // Replaces the text with an empty string
    TFTscreen.text("cookies", 5, 80); // Replaces the text with an empty string
    buttonsOn();

  }else{
    TFTscreen.background(255, 0, 0);
    TFTscreen.stroke(255,255,255); // Clears the previous text
    TFTscreen.text("Access Denied", 5, 40); // Replaces the text with an empty string
    //mySerial.write("Kill");
    tone(A0, 100, 500); // Incorrect tag
  }
  }else{
   delay(100);
    if(questionAnswered == true){
      buttonsOff();
      if (questionRight == true){
        TFTscreen.background(255, 0, 0);
        TFTscreen.stroke(255,255,255); // Clears the previous text
        TFTscreen.text("Door Opened", 5, 40); // Replaces the text with an empty string
        //mySerial.write("Open door");
        restartProgram();
      }else{
        TFTscreen.background(255, 0, 0);
        TFTscreen.stroke(255,255,255); // Clears the previous text
        TFTscreen.text("Wrong Answer Try Again", 5, 40); // Replaces the text with an empty string
        delay(1000);
        TFTscreen.background(255, 0, 0);
        TFTscreen.stroke(255,255,255); // Clears the previous text
        TFTscreen.text("Whats The Password?", 5, 20); // Replaces the text with an empty string
        TFTscreen.text("snow", 5, 40); // Replaces the text with an empty string
        TFTscreen.text("angels", 5, 60); // Replaces the text with an empty string
        TFTscreen.text("cookies", 5, 80); // Replaces the text with an empty string
       // mySerial.write("kill");
        buttonsOn();
      }
      
    }
   }
  }
}



String byteToStringArray(byte *buffer, byte bufferSize) {
  String arr[bufferSize];
  for (byte i=0; i < bufferSize; i++) arr[i] = String(buffer[i], HEX);
  String str = intArrayToString(arr, bufferSize);
  return str;
}


String intArrayToString(String *intArray, byte bufferSize){
//    String numbers[4];
//    for (int j=0; j < bufferSize; j++) numbers[j] = String(buffer[j], HEX);
    String fullString;
    for (int j=0; j < bufferSize; j++) {
      fullString.concat(intArray[j]);
      fullString.concat(" ");
    }
    return fullString;
}

void answer1() { //ISR for the answer 1 button
 buttonsOff();
}
void answer2() { //ISR for the answer 2 button
  buttonsOff();
  questionRight = true;
}
void buttonsOff(){
 pinMode(answerButton1, OUTPUT); //set player buttons as inputs
 pinMode(answerButton2, OUTPUT);
 questionAnswered = true;
}
void buttonsOn(){
 pinMode(answerButton1, INPUT); //set player buttons as inputs
 pinMode(answerButton2, INPUT);
 questionAnswered = false;

}


String recieveCommand() {
    String command;
    delay(100); //
    if (mySerial.available()) {
      char c = mySerial.read();  //gets one byte from serial buffer
      command += c;
    } //makes the string readString

    if (command.length() > 0) {
      //Serial.println(test); //prints string to serial port out
      //test = ""; //clears variable for new input
      return command; // Returns t or f
    }
    else return "f"; // Returns f if nothing found
}


void restartProgram() {
  RFIDValid = "f";
  questionRight = false;
  questionAnswered = false;
  faceDetected = false;
  delay(1000); // 5000 in final product
  setup();
}
