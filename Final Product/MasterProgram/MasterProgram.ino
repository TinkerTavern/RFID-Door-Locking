//importing servo library
#include <Servo.h>
//initializing servo
Servo doorLock;
int pos = 0;

#include <millisDelay.h>
// RFID Libraries
#include <MFRC522.h>
// TFT Screen Libraries
#include <SPI.h>
#include <TFT.h>
// Bluetooth library (Software serial)
#include <SoftwareSerial.h>
// Initializing bluetooth connection
SoftwareSerial mySerial(4, 5); // RX, TX Pins for the HC-05 modules
// TFT Screen pins, configured to be different for compatability
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
  doorLock.attach(A5);
  mySerial.begin(38400);
  Serial.begin(9600); // Initialize serial communications with the PC
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();        // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
  //  for (byte i = 0; i < 6; i++) {
  //    key.keyByte[i] = 0xFF;
  //  }
  attachInterrupt(0, answer1, FALLING); // Specifying which interrupt pins correspond to which pin, and specifying the ISRs
  attachInterrupt(1, answer2, FALLING);

  pinMode(answerButton1, INPUT); // Set player buttons as inputs
  pinMode(answerButton2, INPUT);

  TFTscreen.background(255, 0, 0);
  TFTscreen.stroke(255, 255, 255);
  TFTscreen.text("Waiting for face", 5, 40); // Replaces the text with an empty string
}

// Main function which will be looped through
void loop() {
  // Checking if a face has been detected from the slave arduino
  if (!faceDetected) {
    String newString;
    if (Serial.available()) {
      while (Serial.available()) {
        delay(10);  // Small delay to allow input buffer to fill
        char c = Serial.read();  // Gets one byte from serial buffer
        newString += c;
      } // Makes the string readString
      // If a face has been detected, start the timer
      if (newString.length() > 0) {
        if (newString == "True") {
          tone(A0, 3000, 500); // Correct tag noise
         
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
  // Check the timer to see if it has run out, if not then update said timer.
  else {
    checkTimer();
    updateTimer();
    // If they have not scanned a correct RDIF tag
    if (RFIDValid == "f") {
      String sensorVal;
      // Look for new cards
      if (! rfid.PICC_IsNewCardPresent())
        return;

      // Verify if the NUID has been readed
      if ( ! rfid.PICC_ReadCardSerial())
        return;

      //      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
      //
      //      // Check is the PICC of Classic MIFARE type
      //      if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      //          piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      //          piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
      //        return;
      //      }

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

      // If they have scanned a correct RFID tag then we need to swap to question mode
      if (RFIDValid == "t") {
        updateScreen("Access Granted", 15, 5, 40);
        tone(A0, 3000, 500); // Correct tag noise
        delay(1000);
        checkTimer(); // Checking if the time is up
       updateScreen("What is the answer ", 21 , 5, 20);
          TFTscreen.text("to the universe?", 5, 40);
          TFTscreen.text("1", 50, 60); // Replaces the text with an empty string
          TFTscreen.text("0", 50, 80); // Replaces the text with an empty string
        buttonsPressed = false;

      }
      else if (RFIDValid == "r") {
        // If the return was "r" restart the program after the alarm is turned on
        // As there have been 3 incorrect rfid scans
        startAlarm();
      }
      else {
        // Else, if the RFID was not valid then, display access denied and play an alarm
        updateScreen("Access Denied", 15, 5, 40);
        tone(A0, 100, 500); // Incorrect tag
      }
    }
    // If the RFID tag has been answered as correct, then check the question answer
    else {
      delay(10);
      // If the answer is of length 8 then it is a full answer and needs to be evaluated
      if (binaryAnswer.length() == 8) {
        //      String Answer="";

        char Answer[8];
        // Loops through the answer and adds it to a char array
        // To be writted to the slave arduino for checking
        for (int i = 0 ; i < 8; i++) {
          Answer[i] = binaryAnswer.charAt(i);
          Answer[i + 1] = '\0';
        }
        Serial.println(Answer);
        mySerial.write(Answer);
        // A delay of 300 to allow the slave arduino to recieve,process and send a response
        delay(300);
      }
      // If there is a message from the slave
      if (mySerial.available()) {
        String result = recieveCommand();
        binaryAnswer = "";
        // If the slave returned true then the "door" will be opened
        // A nice tune will also be played and the program will restart after some time
        if (result == "t") {
          for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
            doorLock.write(pos);              // tell servo to go to position in variable 'pos'
            delay(2);                       // waits 15ms for the servo to reach the position
          }
          updateScreen("Door Opened", 12, 5 , 40);
          delay(500);
          tone(A0, 300, 500);
          delay(200);
          tone(A0, 900, 500);
          delay(200);
          tone(A0, 1200, 500);
          restartProgram();
          for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
            doorLock.write(pos);              // tell servo to go to position in variable 'pos'
            delay(2);                       // waits 15ms for the servo to reach the position
          }
        }
        else if (result == "r") {
          // If the return was "r" restart the program after the alarm is turned on
          // As there have been 3 incorrect question answers
          startAlarm();
        }
        else {
          // If the slave returns anything other then true
          // output that the answer is wrong and reset the questions
          delay(1000);
          binaryAnswer = "";
          updateScreen("Wrong Answer! Try Again", 25, 5, 40);
          delay(1000);
          // Re-writing the screen
          updateScreen("What is the answer ", 21 , 5, 20);
          TFTscreen.text("to the universe?", 50, 40);
          TFTscreen.text("1", 50, 60); // Replaces the text with an empty string
          TFTscreen.text("0", 50, 80); // Replaces the text with an empty string
          binaryAnswer = "";
        }


      }
      // Turn the buttons back on

      if (faceDetected == true) {
        buttonsPressed = false;
      }
    }
  }
}


// This function will take the input from system.read and turn it int oa string array
String byteToStringArray(byte *buffer, byte bufferSize) {
  String arr[bufferSize];
  // Loop through for each integer value and store it in a new array
  for (byte i = 0; i < bufferSize; i++) {
    arr[i] = String(buffer[i], HEX);
  }
  // Pass the new array to the intArrayToString function which will return a string
  String str = intArrayToString(arr, bufferSize);
  return str;
}

// This is used by the above function, to turn a string into an integer
String intArrayToString(String *intArray, byte bufferSize) {
  // For each Value in the array we loop through and add this to a new string which we then return
  String fullString;
  for (int j = 0; j < bufferSize; j++) {
    fullString.concat(intArray[j]);
    fullString.concat(" ");
  }
  return fullString;
}


void answer1() { // ISR for the answer 1 button
  if (buttonsPressed == false) {
    binaryAnswer += "1";
    buttonsPressed = true;
  }
}

void answer2() { // ISR for the answer 2 button
  if (buttonsPressed == false) {
    binaryAnswer += "0";
    buttonsPressed = true;
  }
}

// Here we have a function used to recieve information from the slave arduino
String recieveCommand() {
  String command;
  delay(100);
  // If the connection is available (there is data on the connection)
  if (mySerial.available()) {
    char c = mySerial.read();  // Gets one byte from serial buffer
    command += c; // Adds byte to a string
  }
  // If the string is not null return the string
  if (command.length() > 0) {

    return command; // Returns the string
  }
  else return "f"; // Returns f if nothing found
}

// Update the timer function, this updates the timer on the led screen
void updateTimer() {
  char buff[5];

  int timeBuffer = timer.remaining() % 1000;
  if (timeBuffer < 150) {
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

}

// Function to check if the time is up
void checkTimer() {
  // Boolean value to store if the timer is up
  bool timerCheck = timer.isFinished();
  // If the timer is up, or either the RFID or the question have been done wrong 3 times then
  if (timerCheck) {
    // Repeat an alarm
    startAlarm();
  }
}

// Triggers the alarm by showing text on the screen and triggering the piezo speaker
void startAlarm() {
  // Update the screen
  updateScreen("INTRUDER ALERT!!!!!!", 22, 5, 20);
  for (int i = 0; i < 10; i++) {
    tone(A0, 2000, 500);
    delay(200);
    tone(A0, 3000, 500);
    delay(200);
  }
  // Restart the program
  restartProgram();
}

// This is the function for updating the text on the screen
// It takes as input the text itself, length, x and y
void updateScreen(String text, int textLen, int x, int y) {
  TFTscreen.background(255, 0, 0);
  TFTscreen.stroke(255, 255, 255); // Clears the previous text
  char buf[textLen];
  text.toCharArray(buf, textLen);
  TFTscreen.text(buf, x, y);
}

// This function restarts the program at the end
// It does so by resetting all variables and the screen and waiting for 5 seconds
void restartProgram() {
  RFIDValid = "f";
  binaryAnswer = "";
  faceDetected = false;
  buttonsPressed = true;
  delay(5000);
  TFTscreen.background(255, 0, 0);
  TFTscreen.stroke(255, 255, 255);
  TFTscreen.text("Waiting for face", 5, 40); // Replaces the text with an empty string
}
