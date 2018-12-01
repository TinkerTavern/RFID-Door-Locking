#include <millisDelay.h>

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
String binaryAnswer = "";

TFT TFTscreen = TFT(cs, dc, rst);

MFRC522 rfid(SS_PIN, RST_PIN);   // Create MFRC522 instance.

MFRC522::MIFARE_Key key;

millisDelay timer;
// Init array that will store new NUID 
byte nuidPICC[4];

char sensorPrintout[13];
bool faceDetected = false;
String timeRemaining;
int invalidRFID = 0;
int invalidPassword = 0;

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
  TFTscreen.text("Waiting for face", 5, 40); // Replaces the text with an empty string
//  timer.start(30000);
//  updateTimer();
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
      if (newString == "True"){
        faceDetected = true;
        TFTscreen.background(255, 0, 0);
        TFTscreen.stroke(255,255,255);
        TFTscreen.text("Please scan RFID card", 5, 40); // Replaces the text with an empty string
        if (!timer.isRunning()) timer.start(30000);
        if (timer.isRunning()) timer.restart();
        //timer.restart();
        updateTimer();
      }
    }
  }
  }
  else {
  checkTimer();
  updateTimer();
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
    updateScreen("Access Granted",14 , 5, 40);
    //mySerial.write("Dont Kill");
    tone(A0, 3000, 500); // Correct tag
    delay(1000);
    checkTimer();
    updateScreen("What is the answer to the universe?",35 , 5, 20);
    TFTscreen.text("1", 5, 40); // Replaces the text with an empty string
    TFTscreen.text("0", 5, 60); // Replaces the text with an empty string
    buttonsOn();

  }else{
    updateScreen("Access Denied",13 , 5, 40);
    //mySerial.write("Kill");
    tone(A0, 100, 500); // Incorrect tag
    invalidRFID++;
  }
  }else{
   delay(10);
    if (binaryAnswer.length() == 8){
//      String Answer="";

      char Answer[8];
      for(int i = 0 ; i < 8; i++){
        Answer[i] = binaryAnswer.charAt(i);
        Answer[i+1] = '\0';
      }
      Serial.println(Answer);
      mySerial.write(Answer);
      delay(300);
    }
   
    if(mySerial.available()){ 
      String result = recieveCommand();
      binaryAnswer = "";
      if (result == "t"){
        updateScreen("Door Opened",11 , 5, 40);
        delay(500);
        tone(A0, 300, 500);
        delay(200);
        tone(A0, 900, 500);
        delay(200);
        tone(A0, 1200, 500);
        restartProgram();
      }else{
        delay(1000);
        binaryAnswer = "";
        updateScreen("Wrong Answer! Try Again",23 , 5, 40);
        invalidPassword ++;
        delay(1000);
        updateScreen("What is the answer to the universe?",35 , 5, 20);
        TFTscreen.text("1", 50, 40); // Replaces the text with an empty string
        TFTscreen.text("0", 50, 60); // Replaces the text with an empty string
        binaryAnswer = "";
       // mySerial.write("kill");
      }
    }
    
        buttonsOn();
    
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

    String fullString;
    for (int j=0; j < bufferSize; j++) {
      fullString.concat(intArray[j]);
      fullString.concat(" ");
    }
    return fullString;
}
void answer1() { //ISR for the answer 1 button
 buttonsOff();
 binaryAnswer+= "1";
}
void answer2() { //ISR for the answer 2 button
 buttonsOff(); 
 binaryAnswer+= "0";
}
void buttonsOff(){
 pinMode(answerButton1, OUTPUT); //set player buttons as outputs
 pinMode(answerButton2, OUTPUT);
}
void buttonsOn(){
 pinMode(answerButton1, INPUT); //set player buttons as inputs
 pinMode(answerButton2, INPUT);
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

void updateTimer() {
  char buff[5];
  TFTscreen.fill(255, 0, 0);
  TFTscreen.stroke(255,0,0);
  TFTscreen.rect(0,0,TFTscreen.width(),20);
  timeRemaining= String(timer.remaining()/1000);
  timeRemaining.toCharArray(buff, 5);
  TFTscreen.setTextSize(2);
  TFTscreen.stroke(255,255,255);
  TFTscreen.text(buff,0,0);
  TFTscreen.setTextSize(1);
}

void checkTimer() {
  bool timerCheck = timer.isFinished();
  if (timerCheck || invalidRFID > 2 || invalidPassword > 2) {
    for (int i=0; i<10; i++) {
      tone(A0, 2000, 500);
      delay(200);
      tone(A0, 3000, 500);
      delay(200);
    }
    updateScreen("INTRUDER ALERT!!!!!!", 20, 5, 20);
    restartProgram();
  }
}

void updateScreen(String text, int textLen, int x, int y) {
  TFTscreen.background(255, 0, 0);
  TFTscreen.stroke(255,255,255); // Clears the previous text
  char buf[textLen];
  text.toCharArray(buf, textLen);
  TFTscreen.text(buf, x, y);
}

void restartProgram() {
  RFIDValid = "f";
  binaryAnswer = "";
  faceDetected = false;
  invalidRFID = 0;
  invalidPassword = 0;
  delay(1000); // 5000 in final product
  setup();
}
