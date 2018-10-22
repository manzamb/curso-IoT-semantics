
#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Requires:
# - Running Raspberry Pi with Python
# - ADS1015 Analog/Digital Convertor: http://www.adafruit.com/products/1083
# - Adafruit Library: https://github.com/adafruit/Adafruit-Raspberry-Pi-Python-Code/tree/master/Adafruit_ADS1x15
# - TI LM35 Temperature Sensor: http://www.mikrocontroller.net/part/LM35

import time, sys
from Adafruit_ADS1x15 import ADS1x15

class TempSensor:
	
	adc = None

	def __init__(self):
	    ADS1015 = 0x00
	    self.adc = ADS1x15(address=0x48, ic=ADS1015, debug=True)
	
	def read(self):
	    milli = self.adc.readADCSingleEnded(2, 6144, 250)
	    celsius = (milli-50)/10
	    return celsius

	def write(self, filename, celsius):
		f = open(filename, 'a+')
		f.write('{0};{1}\n'.format(round(time.time(), 0), celsius))
		f.close()

sensor = TempSensor()
while(True):
	celsius = sensor.read()
	sensor.write("sensor.log", celsius)

	time.sleep(3600)
