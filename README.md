# RFID-Door-Locking
## Year 1 Semester 1 Coursework 2: An RFID Door lock with facial detection

This coursework involved only one main task: To create a program that uses 2 Arduinos communicating
together in some way.

With this, we decided to create an RFID door locking system, that uses facial detection to trigger.

This works by:
1) Using a python program, linked to OpenCV to detect a face in a live feed of a webcam
2) Once a face is detected, a serial command is sent to the master Arduino, to start the code
3) When the code starts, a 30 second timer starts, and the user is asked to scan a valid RFID tag
4) If the user scans a valid tag, the user is then asked to answer a question (the answer being 00101010, 42 in binary)
5) If they enter that correctly and in sufficient time, then the "door" is unlocked, and a noise is made

The RFID tags and question answers are sent to a slave Arduino, which then checks to see if it's valid, and sends that back
If the user enters 3 incorrect passwords; scans an invalid card 3 times; or doesn't finish the tasks in time, the user will be
locked out and an alarm will go off.


All of the information is shown on a small TFT screen, controlled by the master Arduino
This information includes the timer and the text telling the user what to do
