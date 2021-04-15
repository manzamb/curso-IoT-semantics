#include <Arduino.h>

/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  For example in Internet Day
*/
const int bombillopin = D0;
const int temperaturePin = A0;

// the setup function runs once when you press reset or power the board
void setup() {
  //Injicializar puerto de comunicaciones para lectura serial
  Serial.begin(115000);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(bombillopin, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {

  // CODIGO SENSOR POTENCIOMETRO O FOTOCELDA
  //int sensorValue = analogRead(A0);
  // print out the value you read:
  //Serial.println(sensorValue);

  //CODIGO SENSOR DE TEMPERARTURA
  //recibe la temperatura del método Para el TMP36
 float temperature = analogRead(temperaturePin)*(3.3/1024);   
 temperature = temperature*100 - 50; 
 //impresión en la pantalla seria del IDE arduino                                              
 Serial.println(temperature);
 
 if(temperature>26)
    digitalWrite(bombillopin, HIGH);   // turn the LED on (HIGH is the voltage level)
  else
    digitalWrite(bombillopin, LOW);    // turn the LED off by making the voltage LOW

  delay(1000);                       // wait for a second
}