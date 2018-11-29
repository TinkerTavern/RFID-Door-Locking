

#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3);
String test;
String validIDs[2] = {"1d 53 fe 9c ","bd a5 0 9d "};


void setup() {
  Serial.begin(9600);
  mySerial.begin(38400);

  //mySerial.println("testSlave");
  //pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, LOW);
  delay(10);

  if (mySerial.available()) {
    while (mySerial.available()) {
      delay(10);  //small delay to allow input buffer to fill

      char c = mySerial.read();  //gets one byte from serial buffer
      test += c;
    } //makes the string readString

    if (test.length() > 0) {
      testUID(test);
      test = ""; //clears variable for new input
    }
  }

  // Serial.print(test);
}

void testUID(String ID) {
  bool valid = false;
  for (String testID: validIDs) {
    if (ID == testID) {
      valid = true;
    }
  }
  if (valid) {
    mySerial.write("t"); // Doesn't send t the first time for some reason??
  }
  else {
    mySerial.write("f");
  }
}
