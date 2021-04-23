#include <Arduino.h>


//pin en el que se conecta la salida Vout o pi medi del sensor de Fotocelda
const int fotoceldaPin = A3; 
const int ledPin = 4;
int brilloLed;

//metodo cliente
void setup()
{
  //Velocidad del puerto
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
}

//metodo repetitivo
void loop()                    
{
 //recibe la temperatura del método
 float valorLuz = analogRead(fotoceldaPin);   
 //impresión en la pantalla seria del IDE arduino                                              
 Serial.print("Luz Percibida: ");
 Serial.println(valorLuz);

//espera en milisegundos para volver a tomar la temperatura 
 delay(1000);
}