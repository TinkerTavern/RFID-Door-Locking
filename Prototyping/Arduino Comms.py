import cv2
import sys
import serial
import time

cascPath = "data/haarcascade_frontalface_alt.xml"#sys.argv[1]
faceCascade = cv2.CascadeClassifier(cascPath)

video_capture = cv2.VideoCapture(0)

ser = serial.Serial('com3', 9600) # USB Serial for arduino
time.sleep(2)

counter = 0
limit = 50

while True:
    found = False
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
        cv2.rectangle(frame, (x, y), (x + width, y + height), (255, 0, 255), 2) # Maybe different colours?
        if width > 100:
            counter += 1
            if counter > limit:
                found = True
        else:
            counter = 0
    # Display the resulting frame
    cv2.imshow('Video', frame)

    
    if found:
        ser.write(b"True")
        counter = 0
        found = False
        time.sleep(60)
        #print(ser.readline())
        
    if len(faces) == 0:
        counter = 0
    #print(ser.readline())
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
# When everything is done, release the capture
video_capture.release()
cv2.destroyAllWindows()
