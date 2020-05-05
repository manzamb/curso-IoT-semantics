import RPi.GPIO as GPIO
import time

GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)

led = 21

GPIO.setup(led,GPIO.OUT)

while True:
    GPIO.output(led,1)
    time.sleep(1)
    GPIO.output(led,0)
    time.sleep(1)
    
GPIO.cleanup()
