#include <Arduino.h>


//pin en el que se conecta la salida Vout o pi medi del sensor de Fotocelda
const int fotoceldaPin = A2; 
const int ledPin = 9;
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
 //Aqui se verifica el umbral de luz medido por la fotocelda
 //como no tengo una resustencia de 10kh y coloque una de 4.5 kh 
 //se debe encontrar el mínimo voltaje y el máximo voltaje para poder mapearlo
 //correctamente y encender el led de acuerdo a la luz presente con PWM sobre el led
 //Para se usa la función MAP() 
 int valminfotocelda = 231;      //originalmente sería 0
 int valmayorfotocelda = 820;  //originalmente seria 1023
 int valminBrilloLed = 0;
 int valmaxbrilloLed = 255;

 //recibe la temperatura del método
 float valorLuz = analogRead(fotoceldaPin);   
 //impresión en la pantalla seria del IDE arduino                                              
 Serial.print("Luz Percibida: ");
 Serial.println(valorLuz);

//Se resta el valor percibido para calcular el mapeo inverso
//cuando se apague la luz el led enciende el máximo
 brilloLed=map(valmayorfotocelda-valorLuz, valminfotocelda, 
              valmayorfotocelda,valminBrilloLed, valmaxbrilloLed);
 
  //impresión en la pantalla seria del IDE arduino                                              
 Serial.print("Brillo Mapeado: ");
 Serial.println(brilloLed);

 //Escribe en el led en formato PWM
 analogWrite(ledPin, brilloLed);
 if(valorLuz>200)
    digitalWrite(ledPin, HIGH);   // turn the LED on (HIGH is the voltage level)
  else
    digitalWrite(ledPin, LOW);    // turn the LED off by making the voltage LOW
  

//espera en milisegundos para volver a tomar la temperatura 
 delay(1000);
}


