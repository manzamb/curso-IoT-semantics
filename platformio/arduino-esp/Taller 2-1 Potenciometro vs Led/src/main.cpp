#include <Arduino.h>


//pin en el que se conecta la salida Vout o pi medi del sensor de Fotocelda
const int PotenciometroPin = A5; 
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
 //PWM es Pulse Width Modulation convierte una señal digital en una señal analógica 
 //cambiando la cantidad de tiempo que se mantiene encendida o apagada
 //El PWM, Pulse Width Modulation o Modulación por Ancho de Pulsos, sirve para variar 
 // la energía recibida por un dispositivo electrónico variando rápidamente la energía que este recibe, 
 //cambiando entre apagado y encendido. Una variación en el PWM produce un cambio en el Duty Cycle.
 //El Duty Cycle es el tiempo que la señal está activa frente al tiempo que la señal está apagada. 
 //Con un Duty Cycle del 50% la señal estará activa la mitad del tiempo, mientras que la otra mitad del tiempo estará apagada. Si el Duty Cycle es del 100%, la señal estará activa durante todo el tiempo.
 //ver mas (https://hardzone.es/2018/03/11/uso-pwm-pc/)
 //Para eso se usa la función MAP() 
 int valminPotenciometro = 20;       //Cambiar por el valor mínimo experimental
 int valmayorPotenciometro = 907;  //Cambiar por el valor máximo experimental
 int valminBrilloLed = 0;
 int valmaxbrilloLed = 255;

 //recibe la temperatura del método
 float voltajePercibido = analogRead(PotenciometroPin);   
 //impresión en la pantalla seria del IDE arduino                                              
 Serial.print("Voltaje Percibido: ");
 Serial.println(voltajePercibido);

//Se resta el valor percibido para calcular el mapeo inverso
//cuando se apague la luz el led enciende el máximo
 brilloLed=map(voltajePercibido, valminPotenciometro, 
              valmayorPotenciometro,valminBrilloLed, valmaxbrilloLed);
 
  //impresión en la pantalla seria del IDE arduino                                              
 Serial.print("Brillo Mapeado: ");
 Serial.println(brilloLed);

 //Escribe en el led en formato PWM 0-255
 analogWrite(ledPin, brilloLed);
 //if(voltaje>50)
 //   digitalWrite(ledPin, HIGH);   // turn the LED on (HIGH is the voltage level)
 // else
 //   digitalWrite(ledPin, LOW);    // turn the LED off by making the voltage LOW
  

//espera en milisegundos para volver a tomar la temperatura 
 delay(1000);
}