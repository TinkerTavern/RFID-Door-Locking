//char string[15];
//char state[1];

boolean triggered = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(13, OUTPUT);
}

//void loop() {
//  // put your main code here, to run repeatedly:
//  while (Serial.available() > 0) {
//      digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
//      delay(100);                       // wait for a second
//      digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
//      delay(100); 
//    Serial.readBytes(string,15);
//    Serial.println(string);
//  }
//}


void loop() {
  while (!triggered) checkDetection();

}

void checkDetection() {
    while (Serial.available() > 0) {
      int value = Serial.read();
      digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(100);                       // wait for a second
      digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
      delay(100); 
  }
}
