#include <Arduino.h>


//Leer temperatura con un TMP36
//pin en el que se conecta la salida Vout o pi medi del sensor TMP36
const int temperaturePin = A0; 
const int umbraltemperatura = 20;
const int ledpin = 3;

//calcula la temperatura dependiendo del voltaje recibido
float getVoltage(int pin){
 return (analogRead(pin) * .004882814);
}
//metodo cliente
void setup()
{
  //Inicializar el puerto serial
  Serial.begin(115200);             
}

//metodo repetitivo
void loop()                    
{
 //recibe la temperatura del método
 
 float temperature = getVoltage(temperaturePin);  
 temperature = (temperature - .5) * 100;      
 //float temperature = ( analogRead(temperaturePin) * (500.0 / 1023.0) ) - 50.0;
 //impresión en la pantalla seria del IDE arduino                                              
 Serial.println(temperature);

 //Verifico el umbral de temperatura
 if(temperature>umbraltemperatura)
    digitalWrite(ledpin, HIGH);   // turn the LED on (HIGH is the voltage level)
  else
    digitalWrite(ledpin, LOW);    // turn the LED off by making the voltage LOW

//espera en milisegundos para volver a tomar la temperatura 
 delay(1000);
}

