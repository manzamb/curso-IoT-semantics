#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 2 // Pin de datos del sensor DHT11
#define DHTTYPE    DHT11     // DHT 11

DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;

//Leer temperatura con un TMP36
//pin en el que se conecta la salida Vout o pi medi del sensor TMP36
//const int temperaturePin = A0; 
const int umbraltemperatura = 20;
const int ledpin = 3;

//calcula la temperatura dependiendo del voltaje recibido
//float getVoltage(int pin){
// return (analogRead(pin) * .004882814);
//}
//metodo cliente
void setup()
{
  //Inicializar el puerto serial
  Serial.begin(115200); 
  // Initialize device.
  dht.begin();
  Serial.println(F("DHT11 Inicializando sensor de temperatura"));            
}

//metodo repetitivo
void loop()                    
{
 float temperature;
 //recibe la temperatura del método
 // Delay between measurements.
  delay(delayMS);
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    temperature = event.temperature;
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C"));
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    Serial.print(F("Humidity: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
  }

 //float temperature = getVoltage(temperaturePin);  
 //temperature = (temperature - .5) * 100;      
 //float temperature = ( analogRead(temperaturePin) * (500.0 / 1023.0) ) - 50.0;
 //impresión en la pantalla seria del IDE arduino                                              
 //Serial.println(temperature);

 //Verifico el umbral de temperatura
 if(temperature>umbraltemperatura) {
    digitalWrite(ledpin, HIGH);   // turn the LED on (HIGH is the voltage level)
    Serial.println("temperaytura: ");
    Serial.println(temperature);
    Serial.println("Encender led");
    }
  else
    digitalWrite(ledpin, LOW);    // turn the LED off by making the voltage LOW

//espera en milisegundos para volver a tomar la temperatura 
 delay(1000);
}