In order to get this program to fully work, you need to install a few libraries:

For arduino, the RFID library (MFRC522) needs to be installed. This can be installed by opening the arduino IDE
Going to sketch
Selecting include library
Selecting the rfid-master.zip file
Then, when calling it, you use #include <MFRC522.h>
The millisDelay library also needs to be installed. This can be done by using the same process, but is called by using
#include <millisDelay.h>
Both of which are included in this folder.


For Python
1) OpenCV-Python needs to be installed for the face detection. Once it is, you can call it using import cv2
2) PySerial needs to be installed for the PC to arduino comms. Once it is, you can call it using import serial

To install a python library:
If you have set up pip in your PATH, you can simply run pip3 install #LIBRARY NAME#
If you don't, then you need to navigate to your python install folder, then to scripts. 
From there, you can run the same pip command.
These need to be installed through pip, so can not be included in this folder.