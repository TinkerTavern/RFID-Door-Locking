

#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3);
String test;
void setup() {
  Serial.begin(9600);
  mySerial.begin(38400);

  mySerial.println("testSlave");
  //pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);

  if (mySerial.available()) {
    while (mySerial.available()) {
      delay(10);  //small delay to allow input buffer to fill

      char c = mySerial.read();  //gets one byte from serial buffer
      test += c;
    } //makes the string readString

    if (test.length() > 0) {
      Serial.println(test); //prints string to serial port out
      test = ""; //clears variable for new input
    }
  }

  // Serial.print(test);



}
