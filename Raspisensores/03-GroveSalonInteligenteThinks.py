#!/usr/bin/env python

# Ejemplo de Salon Inteligente con Grove Pi
# creado por: Miguel Angel Nino Zambrano
# manzamb@unicauca.edu.co
import time
import grovepi

# librerias para ThinkSpeak
import urllib2 as ul
import json
import time
import os

# Set the global variables to collect data once every 15 seconds and
# update the channel once every 2 minutes
lastConnectionTime = time.time() 	# Track the last connection time
lastUpdateTime = time.time() 		# Track the last update time
postingInterval = 15 				# Post data once every 2 minutes
updateInterval = 15 				# Update once every 15 seconds

# Replace YOUR-CHANNEL-WRITEAPIKEY with your channel write API key
writeAPIkey = "YXMRSP0G6NDXUCW8"
# Replace YOUR-CHANNELID with your channel ID
channelID = "431529"
# ThingSpeak server settings
url = "https://api.thingspeak.com/channels/"+channelID+"/bulk_update.json"
messageBuffer = []

# Establece los pines a ser conectados
bombillopin = 4     # Simula prender el bonbillo. D4
ventiladorpin = 3   # Activa un rele que prenderia el ventilador. D2
temperatutapin = 0  # Lee la temperatura ambiente. A0
sensorluzpin = 2    # Lee el sensor de luz. A2

# Establecer el modo de lectura y escritura de cada pin
grovepi.pinMode(bombillopin, "OUTPUT")
grovepi.pinMode(ventiladorpin, "OUTPUT")
grovepi.pinMode(sensorluzpin, "INPUT")
grovepi.pinMode(temperatutapin, "INPUT")

# Variables Globales
umbralLuz = 600				# Es el umbral para encender el bombillo
umbralTemperatura = 25		# Es el umbral para encender el ventilador
luminosidad = 0				# Toma el valor en voltaje
temperatura = 0				# Toma el valor en grados

# Funciones de apoyo
'''Function to send the POST request to 
   ThingSpeak channel for bulk update.'''
def httpRequest():
    global messageBuffer
    data = json.dumps({'write_api_key': writeAPIkey,'updates': messageBuffer}) # Format the json data buffer
    req = ul.Request(url = url)
    requestHeaders = {"User-Agent":"mw.doc.bulk-update (Raspberry Pi)","Content-Type":"application/json","Content-Length":str(len(data))}
    for key,val in requestHeaders.iteritems(): # Set the headers
            req.add_header(key,val)
    req.add_data(data) # Add the data to the request
    # Make the request to ThingSpeak
    try:
            response = ul.urlopen(req) # Make the request
            print response.getcode() # A 202 indicates that the server has accepted the request
    except ul.HTTPError as e:
            print e.code # Print the error code
    messageBuffer = [] # Reinitialize the message buffer
    global lastConnectionTime
    lastConnectionTime = time.time() # Update the connection time


def getData():
    #Estableciendo que variables son Globales
    global luminosidad      
    global temperatura
    global bombillo
    global ventilador
    
    releer = True
    
    while releer:
		try:
			# Obtener valor de luz
			luminosidad = grovepi.analogRead(sensorluzpin)
		
			# Obtener el valor de la temperatura
			temperatura = grovepi.temp(temperatutapin,'1.1')
			
			#Terminar bucle correctamente
			releer = False
		except: 
			print "Error en la lectura de sesnores...volviendo a leer"
			time.sleep(.5)
    
    #Estado del Bombillo
    if (UmbraldeLuz()):
        bombillo = 1
    else:
        bombillo = 0

    #Estado del Ventilador
    if (UmbraldeTemperatura()):
        ventilador = 1
    else:
        ventilador = 0
    
    #Imprimir Valores
    ImprimirValorSensores();
            
    return temperatura,luminosidad, bombillo, ventilador

def updatesJson():
    '''Function to update the message buffer
    every 15 seconds with data. And then call the httpRequest 
    function every 2 minutes. This examples uses the relative timestamp as it uses the "delta_t" parameter. 
    If your device has a real-time clock, you can also provide the absolute timestamp using the "created_at" parameter.
    '''

    global lastUpdateTime
    message = {}
    message['delta_t'] = int(round(time.time() - lastUpdateTime))
    Temp,Luz, Bom, Vent = getData()
    message['field1'] = Temp
    message['field2'] = Bom
    message['field3'] = Vent
    message['field4'] = Luz
    global messageBuffer
    messageBuffer.append(message)
    
    # If posting interval time has crossed 2 minutes update the ThingSpeak channel with your data
    if time.time() - lastConnectionTime >= postingInterval:
            httpRequest()
    lastUpdateTime = time.time()
    
    
def UmbraldeLuz():
    if luminosidad < umbralLuz:
        # cambiar el estado de la luz a prendido
        grovepi.digitalWrite(bombillopin,1)
        return True
    else:
        # Cambiar el estado de la luz a apagado
        grovepi.digitalWrite(bombillopin,0)
        return False
    

def UmbraldeTemperatura():
    if temperatura > umbralTemperatura:
        # cambiar el estado de la luz a prendido
        grovepi.digitalWrite(ventiladorpin,1)
        return True
    else:
        # Cambiar el estado de la luz a apagado
        grovepi.digitalWrite(ventiladorpin,0)
        return False
        
def ImprimirValorSensores():
    #Imprimir los valores sensados
    print("========================================")
  
    #Temeratura
    print("Temperatura (C): %.2f" %(temperatura))

    #Luminosidad
    print("Luminosidad (L): %.2f" %(luminosidad))

    #Estado del Bombillo
    if (UmbraldeLuz()):
        print("Bombillo Encendido")
    else:
        print("Bombillo Apagado")

    #Estado del Ventilador
    if (UmbraldeTemperatura()):
        print("Ventilador Encendido")
    else:
        print("Ventilador Apagado")

    
    
if __name__ == "__main__":  # To ensure that this is run directly and does not run when imported 
	while True:
		try:
			if time.time() - lastUpdateTime >= updateInterval:
				print("========================================")
				print("Enviando datos a ThinkSpeak...")
				updatesJson()
			else:
				getData()
				time.sleep(0.5)
				
		except KeyboardInterrupt:	# Turn LED off before stopping
			grovepi.digitalWrite(bombillopin,0)
			grovepi.digitalWrite(ventiladorpin,0)
			break
		except IOError:				# Print "Error" if communication error encountered
			print ("Error")
