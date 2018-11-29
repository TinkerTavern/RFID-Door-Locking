#include <SoftwareSerial.h>

SoftwareSerial mySerial(2,3);
int incomingByte = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  mySerial.begin(38400);
  pinMode(9,OUTPUT);
  digitalWrite(9,HIGH);
  Serial.println("Enter AT Commands:");
  
}

void loop() {
  // put your main code here, to run repeatedly:
//  if (mySerial.available()) Serial.write(mySerial.read());
//
//  if (Serial.available()) mySerial.write(Serial.read());
    Serial.println(mySerial.available());
    if (mySerial.available() > 0) {
                // read the incoming byte:
                incomingByte = mySerial.read();

                // say what you got:
                Serial.print("I received: ");
                Serial.println(incomingByte, DEC);
        }  
}
