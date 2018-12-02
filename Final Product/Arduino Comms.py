import cv2
import sys
import serial # Python serial library
import time
import random

cascPath = "data/haarcascade_frontalface_alt.xml"
faceCascade = cv2.CascadeClassifier(cascPath)

video_capture = cv2.VideoCapture(0)

ser = serial.Serial('com3', 9600) # USB Serial for arduino
time.sleep(2) # Sleeps for 2 seconds to allow for comms to set up

counter = 0
limit = 50 # Waits for 50 frames of the person being in view before setting off

while True:
    found = False
    # Capture frame-by-frame
    ret, frame = video_capture.read()

    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    faces = faceCascade.detectMultiScale(gray, scaleFactor=1.1, minNeighbors=5, minSize=(30, 30))
    
    # Draw a rectangle around the faces
    colourArray = [[255,0,0],[0,255,0],[0,0,255],[255,255,0],[0,255,255],[255,0,255],[255,255,255],[0,0,0]]
    colourIndex = 0
    # Starts off red
    for (x, y, width, height) in faces:
        colour = colourArray[colourIndex]
        cv2.rectangle(frame, (x, y), (x + width, y + height), (colour), 2) # Maybe different 
        if width > 100:
            counter += 1
            if counter > limit:
                found = True
        else:
            counter = 0
            
        if colourIndex >= len(colourArray)-1:
            colourIndex = 0
        else:
            colourIndex += 1
            
    # Display the resulting frame
    cv2.imshow('Face Detection Cam', frame)

    
    if found:
        ser.write(b"True")
        counter = 0
        found = False
        time.sleep(30) # Timer to stop the person from being seen 
        
    if len(faces) == 0:
        counter = 0
        
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
    
# When everything is done, release the capture
video_capture.release()
cv2.destroyAllWindows()
