import cv2
import sys
import time as t
import numpy as np
import serial


cascPath = "data/haarcascade_frontalface_alt.xml"#sys.argv[1]
faceCascade = cv2.CascadeClassifier(cascPath)

video_capture = cv2.VideoCapture(0)

timearray = np.zeros(1000)
index = 0
detection = 0
limit = 30

ser = serial.Serial('com3', 9600) # USB Serial for arduino
while True:
    start = t.time()
    # Capture frame-by-frame
    ret, frame = video_capture.read()

    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    faces = faceCascade.detectMultiScale(
        gray,
        scaleFactor=1.1,
        minNeighbors=5,
        minSize=(30, 30),
        #flags=cv2.cv.CV_HAAR_SCALE_IMAGE
    )
    
    # Draw a rectangle around the faces
    for (x, y, width, height) in faces:
        cv2.rectangle(frame, (x, y), (x + width, y + height), (0, 255, 0), 2)
        if width > 100:
            detection += 1
            if detection > limit:
                ser.write(b"Face detected!")
        else:
            detection = 0
    # Display the resulting frame
    cv2.imshow('Video', frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
    end = t.time()
    if index < 1000:
        timearray[index] = end-start
    else:
        break
    index += 1
# When everything is done, release the capture
video_capture.release()
cv2.destroyAllWindows()
print(1/(np.average(timearray)))
