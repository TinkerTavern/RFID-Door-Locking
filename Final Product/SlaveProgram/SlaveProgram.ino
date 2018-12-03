
// Including the library for soft serial
#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3); // RX, TX Pins for the HC-05 modules
// Initializing variables
String input; // To store the input
String validIDs[2] = {"1d 53 fe 9c ","bd a5 0 9d "}; // The valid RDIF UIDs
int RFIDWrong; // How many times the RFID has been scanned wrong
int questionWrong; // Jow many times the question has been answered wrong

void setup() {
  // Establish connections
  Serial.begin(9600);
  mySerial.begin(38400);

}

void loop() {
  input = ""; // Clears variable for new input
  delay(10);
  //if there is data
  if (mySerial.available()) {
    // While data is comming, add the data to the input string
    while (mySerial.available()) {
      delay(10);  // Small delay to allow input buffer to fill

      char c = mySerial.read();  // Gets one byte from serial buffer
      input += c;
    } // Makes the string readString
  Serial.println(input);
   // If the input length > 9 then it is for the RFID. therefor check the RDIF UID
    if (input.length() > 9) {
      testUID(input);
    }

    // If it is not over 9 then it is for the question
    else{
      if (input == "00101010"){ // Checks if the input is 42 in binary
        mySerial.write("t"); // Writes true
        Serial.println("t");
      }
      else{
        questionWrong ++; // Increments the amount of wrong questions
        returnFalse();
      }
    }
  }
}

// This funciton will test the inputted UID against the ones stored
void testUID(String ID) {
  bool valid = false;
  // For each correct ID compare them to the given ID
  for (String testID: validIDs) {
    if (ID == testID) {
      valid = true;
    }
  }
  // If it is valid then write true
  if (valid) {
            Serial.println("t");

    mySerial.write("t"); 
  }
  else {
    // If it is not increment the amount of wrong RFIDs , and return 
   RFIDWrong ++;
   returnFalse();
   }
}


void returnFalse(){
  if ((RFIDWrong > 2) || (questionWrong > 2)){
    RFIDWrong = 0;
    questionWrong = 0;
    mySerial.write("r");
            Serial.println("r");
  }
  else{
    mySerial.write("f");
            Serial.println("f");
  }
}
