#include <millisDelay.h>

// RFID Libraries
#include <SPI.h>
#include <MFRC522.h>
// TFT Screen Libraries
#include <SPI.h>
#include <TFT.h>
//bluetooth libraries
#include <SoftwareSerial.h>
//initializing bluetooth connection
SoftwareSerial mySerial(4, 5);
// TFT Screen pins, configured to be different
#define cs 8
#define dc 7
#define rst 6
// RFID Pins, not changed
#define RST_PIN 9
#define SS_PIN 10
int answerButton1 = 2;
int answerButton2 = 3;
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

// A boolean value to store whether the buttons have been pressed
bool buttonsPressed = true; //starts as true as we dont want the user to use buttons at the start

// Setup function initializing variables and starting connections.
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

  pinMode(answerButton1, INPUT); //set player buttons as inputs
  pinMode(answerButton2, INPUT);

  TFTscreen.background(255, 0, 0);
  TFTscreen.stroke(255, 255, 255);
  TFTscreen.text("Waiting for face", 5, 40); // Replaces the text with an empty string
  //  timer.start(30000);
  //  updateTimer();
}

//main function which will be looped through
void loop() {
  //checking if a face has been detected from the slave arduino
  if (!faceDetected) {
    String newString;
    if (Serial.available()) {
    while (Serial.available()) {
      delay(10);  //small delay to allow input buffer to fill
      char c = Serial.read();  //gets one byte from serial buffer
      newString += c;
    } //makes the string readString
    //if a face has been detected, start the timer
    if (newString.length() > 0) {
      if (newString == "True") {
        faceDetected = true;
        TFTscreen.background(255, 0, 0);
        TFTscreen.stroke(255, 255, 255);
        TFTscreen.text("Please scan RFID card", 5, 40); // Replaces the text with an empty string
        if (!timer.isRunning()) timer.start(30000);
        if (timer.isRunning()) timer.restart();

        updateTimer();
      }
    }
  }
  }
  //check the timer to see if it has run out, if not then update said timer.
  else {
    checkTimer();
    updateTimer();
    //if they have not scanned a correct RDIF tag
    if (RFIDValid == "f") {
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


      sensorVal.toCharArray(sensorPrintout, 13); // Converts the string to a char array for display
      mySerial.write(sensorPrintout); // Sends the RFID tag for verification
      delay(200); // Delay to ensure the slave's response is finished before checking
      RFIDValid = recieveCommand();

      //if they have scanned a correct RFID tag then we need to swap to question mode
      if (RFIDValid == "t") {
        updateScreen("Access Granted", 15, 5, 40);
        tone(A0, 3000, 500); // Correct tag
        delay(1000);
        checkTimer(); //checking if the time is up
        updateScreen("What is the answer to the universe?", 37, 5, 20);
        TFTscreen.text("1", 5, 40); // adding the button values onto the screen
        TFTscreen.text("0", 5, 60);
        buttonsPressed = false;

      }
      else if (RFIDValid == "r") {
        //if the return was "r" restart the program after the alarm is turned on
        //as there have been 3 incorrect rfid scans
        startAlarm();
      }
      else {
        //else, if the RFID was not valid then, display access denied and play an alarm
        updateScreen("Access Denied", 15, 5, 40);
        tone(A0, 100, 500); // Incorrect tag
      }
    }
    //if the RFID tag has been answered as correct, then check the question answer
    else {
      delay(10);
      //if the answer is of length 8 then it is a full answer and needs to be evaluated
      if (binaryAnswer.length() == 8) {
        //      String Answer="";

        char Answer[8];
        //loops through the answer and adds it to a char array
        //to be writted to the slave arduino for checking
        for (int i = 0 ; i < 8; i++) {
          Answer[i] = binaryAnswer.charAt(i);
          Answer[i + 1] = '\0';
        }
        Serial.println(Answer);
        mySerial.write(Answer);
        //a delay of 300 to allow the slave arduino to recieve,process and send a response
        delay(300);
      }
      //if there is a message from the slave
      if (mySerial.available()) {
        String result = recieveCommand();
        binaryAnswer = "";
        //if the slave returned true then the "door" will be opened
        //a nice tune will also be played and the program will restart after some time
        if (result == "t") {
          updateScreen("Door Opened", 12, 5 , 40);
          delay(500);
          tone(A0, 300, 500);
          delay(200);
          tone(A0, 900, 500);
          delay(200);
          tone(A0, 1200, 500);
          restartProgram();
        }
        else if (result == "r") {
          //if the return was "r" restart the program after the alarm is turned on
          //as there have been 3 incorrect question answers
          startAlarm();
        }
        else {
          //if the slave returns anything other then true
          // out put that the answer is wrong and reset the questions
          delay(1000);
          binaryAnswer = "";
          updateScreen("Wrong Answer! Try Again", 25, 5, 40);
          delay(1000);
          //re writing the screen
          updateScreen("What is the answer to the universe?", 37 , 5, 20);
          TFTscreen.text("1", 50, 40); // Replaces the text with an empty string
          TFTscreen.text("0", 50, 60); // Replaces the text with an empty string
          binaryAnswer = "";
        }


      }
      // turn the buttons back on

      if (faceDetected == true) {
        buttonsPressed = false;
      }
    }
  }
}


// This function will take the input from system.read and turn it int oa string array

String byteToStringArray(byte *buffer, byte bufferSize) {
  String arr[bufferSize];
  //loop through for each integer value and store it in a new array
  for (byte i = 0; i < bufferSize; i++) {
    arr[i] = String(buffer[i], HEX);
  }
  //pass the new array to the intArrayToString function which will return a string
  String str = intArrayToString(arr, bufferSize);
  return str;
}

// This is used by the above function, to turn a string into an integer
String intArrayToString(String *intArray, byte bufferSize) {
  //for each Value in the array we loop through and add this to a new string which we then return
  String fullString;
  for (int j = 0; j < bufferSize; j++) {
    fullString.concat(intArray[j]);
    fullString.concat(" ");
  }
  return fullString;
}


void answer1() { //ISR for the answer 1 button
  if (buttonsPressed == false) {
    binaryAnswer += "1";
    buttonsPressed = true;
  }
}
void answer2() { //ISR for the answer 2 button
  if (buttonsPressed == false) {
    binaryAnswer += "0";
    buttonsPressed = true;
  }
}

// Here we have a function used to recieve information from the slave arduino
String recieveCommand() {
  String command;
  delay(100);
  //if the connection is available (there is data on the connection)
  if (mySerial.available()) {
    char c = mySerial.read();  //gets one byte from serial buffer
    command += c; //adds byte to a string
  }
  //if the string is not null return the string
  if (command.length() > 0) {

    return command; // Returns the string
  }
  else return "f"; // Returns f if nothing found
}

// Update the timer function, this updates the timer on the led screen
void updateTimer() {
  char buff[5];
  TFTscreen.fill(255, 0, 0);
  TFTscreen.stroke(255, 0, 0);
  TFTscreen.rect(0, 0, TFTscreen.width(), 20);
  timeRemaining = String(timer.remaining() / 1000);
  timeRemaining.toCharArray(buff, 5);
  TFTscreen.setTextSize(2);
  TFTscreen.stroke(255, 255, 255);
  TFTscreen.text(buff, 0, 0);
  TFTscreen.setTextSize(1);
}

// Function to check if the time is up
void checkTimer() {
  //boolean value to store if the timer is up
  bool timerCheck = timer.isFinished();
  //if the timer is up, or either the RFID or the question have been done wrong 3 times then
  if (timerCheck) {
    //repeat an alarm
    startAlarm();
  }
}

// Triggers the alarm by showing text on the screen and triggering the piezo speaker
void startAlarm() {
  updateScreen("INTRUDER ALERT!!!!!!", 22, 5, 20);
  for (int i = 0; i < 10; i++) {
    tone(A0, 2000, 500);
    delay(200);
    tone(A0, 3000, 500);
    delay(200);
  }
  //update the screen

  //restart the program
  restartProgram();
}

//this is the function for updating the text on the screen
//it takes as input the text itself, length, x and y
void updateScreen(String text, int textLen, int x, int y) {

  TFTscreen.background(255, 0, 0);
  TFTscreen.stroke(255, 255, 255); // Clears the previous text
  char buf[textLen];
  text.toCharArray(buf, textLen);
  TFTscreen.text(buf, x, y);
}

//this function restarts the program at the end
//it does so by resetting all variables and the screen and waiting a delay
void restartProgram() {
  RFIDValid = "f";
  binaryAnswer = "";
  faceDetected = false;
  buttonsPressed = true;
  delay(1000); // 5000 in final product
  TFTscreen.background(255, 0, 0);
  TFTscreen.stroke(255, 255, 255);
  TFTscreen.text("Waiting for face", 5, 40); // Replaces the text with an empty string
}
