#include <Arduino.h>

/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  For example in Internet Day
*/
const int bombillopin = D3;
const int temperaturePin = A0;
float luminosidad;              //Toma el valor en voltaje
float temperatura;

// the setup function runs once when you press reset or power the board
void setup() {
  //Injicializar puerto de comunicaciones para lectura serial
  Serial.begin(115000);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(bombillopin, OUTPUT);
  pinMode(temperaturePin, INPUT);
}

// the loop function runs over and over again forever
void loop() {

   //leer el sensor de luz
   luminosidad = analogRead(temperaturePin); 
 
   //Lee estado de sensor de Temperatura para GROVE temp
   int B=3975; //Valor del termistor
   temperatura = analogRead(temperaturePin); //Obtencion del valor leido
   float resistance=(float)(1023-temperatura)*10000/temperatura; //Obtencion del valor de la resistencia
   temperatura=1/(log(resistance/10000)/B+1/298.15)-273.15; //Calculo de la temperatura
 //impresión en la pantalla seria del IDE arduino                                              
 Serial.println(temperatura);
 
 if(temperatura>30)
    digitalWrite(bombillopin, HIGH);   // turn the LED on (HIGH is the voltage level)
  else
    digitalWrite(bombillopin, LOW);    // turn the LED off by making the voltage LOW

  delay(1000);                       // wait for a second
}