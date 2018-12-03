
//including the library for soft serial
#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3);
//initializing variables
String input; // to store the input
String validIDs[2] = {"1d 53 fe 9c ","bd a5 0 9d "}; // the valid RDIF UIDs
int RFIDWrong; //how many times the RFID has been scanned wrong
int questionWrong; // how many times the question has been answered wrong

void setup() {
  //establish connections
  Serial.begin(9600);
  mySerial.begin(38400);

}

void loop() {
  input = ""; //clears variable for new input
  delay(10);
  //if there is data
  if (mySerial.available()) {
    //while data is comming, add the data to the input string
    while (mySerial.available()) {
      delay(10);  //small delay to allow input buffer to fill

      char c = mySerial.read();  //gets one byte from serial buffer
      input += c;
    } //makes the string readString
  Serial.println(input);
   //if the input length > 9 then it is for the RFID. therefor check the RDIF UID
    if (input.length() > 9) {
      testUID(input);
    }

    //if it is not over 9 then it is for the question
    else{
      if (input == "00101010"){ // checks if the input is 42 in binary
        mySerial.write("t"); // writes true
        Serial.println("t");
      }
      else{
        questionWrong ++; // increments the amount of wrong questions
        returnFalse();
      }
    }
  }
}

//this funciton will test the inputted UID against the ones stored
void testUID(String ID) {
  bool valid = false;
  //for each correct ID compare them to the given ID
  for (String testID: validIDs) {
    if (ID == testID) {
      valid = true;
    }
  }
  //if it is valid then write true
  if (valid) {
            Serial.println("t");

    mySerial.write("t"); 
  }
  else {
    //if it is not increment the amount of wrong RFIDs , and return 
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
