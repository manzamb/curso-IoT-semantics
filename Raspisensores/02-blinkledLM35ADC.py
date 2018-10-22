import RPi.GPIO as GPIO
import time

# Import the ADS1x15 module.
import Adafruit_ADS1x15

# Or create an ADS1015 ADC (12-bit) instance.
adc = Adafruit_ADS1x15.ADS1015()

# See table 3 in the ADS1015/ADS1115 datasheet for more info on gain.
GAIN = 4
'''
/  ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
//  ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
//  ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
//  ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
//  ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
'''

# example and read_adc function for more infromation.
adc.start_adc(0, gain=GAIN)

GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)

led = 21

GPIO.setup(led,GPIO.OUT)

while True:
    GPIO.output(led,1)
    time.sleep(0.5)
    GPIO.output(led,0)
    time.sleep(0.5)
    
    # Read the last ADC conversion value and print it out
    value = adc.get_last_result()
    volts = value * (5.0 / 1023) * 100
    temperatura = volts / 10
    print('Valor Actual: {0}'.format(value))
    print('Voltaje Actual: {0}'.format(volts))
    print('Temperatura Actual: {0}'.format(temperatura))
    print("_____________________________________________")
    time.sleep(0.5)

# Stop continuous conversion.  After this point you can't get data from get_last_result!
adc.stop_adc()    
GPIO.cleanup()
