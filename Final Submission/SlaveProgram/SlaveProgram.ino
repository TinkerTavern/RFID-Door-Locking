/* Slave Arduino:
  To start connect
  5v ---> EN
  Rx ---> Rx
  Tx ---> Tx
  Send empty sketch to arduino (with Rx/Tx disconnected)
  As arduino starts up hold button on side of arduino
  Open Serial monitor at 38400 Baud and both NL & CR
Commands Sent:
  AT + UART --- this should show 38400
  AT + ROLE? --- this should show 0
  AT + ADDR? -- should give an address, save this for master

Master Arduino:
  To start connect
  5v ---> EN
  Rx ---> Rx
  Tx ---> Tx
  Send empty sketch to arduino (with Rx/Tx disconnected)
  As arduino starts up hold button on side of arduino
  Open Serial monitor at 38400 Baud and both NL & CR
Commands Sent:
  AT + UART --- this should show 38400
  AT + ROLE = 1 --- this should show "ok" and will set it to master
  AT + CMODE = 0 --- connect mode to fixed address
  AT + BIND = Address --- address seperated by commas not colons
        this is attaching the address to be connected to 

now when done disconnect 5v ---> EN 
       connect RX and TX on module to pins you want on arduino */
       
// Including the library for soft serial
#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3); // RX, TX Pins for the HC-05 module
// Initializing variables
String input; 
String validIDs[2] = {"1d 53 fe 9c ","bd a5 0 9d "}; // The valid RDIF UIDs (our library cards)
int RFIDWrong = 0; // How many times the RFID has been scanned wrong
int questionWrong = 0; // How many times the question has been answered wrong

// This is setup , this runs when the arduino turns on and sets up the connecton 
void setup() {
  // Establish connections
  mySerial.begin(38400);
  //setting the pin for the bubble machine
  pinMode(5, OUTPUT);
}

// This is the main loop which the program will repetively loop through
void loop() {
  input = ""; // Clears variable for new input
  delay(10); 
  // If there is data being sent
  if (mySerial.available()) {
    // While data is comming, add the data to the input string
    while (mySerial.available()) {
      delay(10);  // Small delay to allow input buffer to fill
      char c = mySerial.read();  // Gets one byte from serial buffer
      input += c; // Add byte to a string to construct input
    } 
    if (input == "b" ){ // If the input is "b" (stands for bubble) then turn on the bubble machine 
      activateBubbleMachine();
    }
    if (input == "c"){// If the input is "c", (for reCommence), then it will reset program
      reset();
    }
    // If the input length > 9 then it is for the RFID. therefor check the RDIF UID
    if (input.length() > 9) {
      testUID(input);
    }
    // If it is 8 then it is for the question 
    else if (input.length() == 8){
      if (input == "00101010"){ // Checks if the input is 42 in binary
        mySerial.write("t"); // Writes true to the arduino
        mySerial.flush(); // Waits for the message to send 
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
  // Boolean value for whether or not the ID is valid
  bool valid = false;
  // For each correct ID compare them to the given ID
  for (String testID: validIDs) { // For each Valid ID we will check it against the inputted ID
    if (ID == testID) {
      //If it is the same then turn it true
      valid = true;
    }
  }
  // If it is valid then write true
  if (valid) {
    mySerial.write("t");  // Write true
    mySerial.flush(); // Waiting for the message to send
  }
  else {
    // If it is not increment the amount of wrong RFIDs , and return 
   RFIDWrong ++; // Increments the amount of wrong RFIDs
   returnFalse(); // Runs the return false function
   }
}

//This is the reuturn false function which checks whether or not the user has inputted too many wrong attempts
void returnFalse(){
  if ((RFIDWrong > 2) || (questionWrong > 2)){ // If the RFID or Question has been done wrong 3 times then alarm activate
    reset();
    mySerial.write("r"); // Sends r signifying reset
    mySerial.flush();
  }
  else{
    mySerial.write("f"); //sends f signifying wrong answer
    mySerial.flush();
  }
}

void reset(){
  // Resets the variables for the program
  RFIDWrong = 0;
  questionWrong = 0;
}

void activateBubbleMachine() {  
  // Turns on the relay to activate the bubble machine
  reset();
  digitalWrite(5, HIGH);
  delay(7000);
  digitalWrite(5, LOW);
}
