//Este skecht se ha desarrollado para activar el bombillo a partir del umbral
//también el ventilador cuando la temperatura suba por encima de cierto umbral
#include "Arduino.h"

//These constants make it easier to change pin numbers later without editing the whole program
const int bombillopin = 19;     //that’s the red LED for the lamp.
const int ventiladorpin =18;   //that’s the LED that represents the fan.
const int temperaturapin = 17;  //temperaturapin for the temperature sensor

//Analog Inputs ESP 32
const int potenciometro = 34;   //Poteciometro para ejemplo PWM
const int sensorluzpin = 39;    //sensorluzpin for the light sensor

//Variables Globales
int umbralLuz = 612;            //If the light level is lower than 612, the lamp turns on
int umbralTemperatura = 15;     //If the temperature is higher than 15 degrees Celsius, the fan turns on
//We also use variables like luminosidad and temperatura to store the sensor readings
float luminosidad;              //Toma el valor en voltaje
float temperatura;              //Toma el valor en grados
//Finally, the booleans estadoventilador and estadobombillo keep track of the current state of each device
boolean estadoventilador=false; //false = apagado
boolean estadobombillo = false; //false = apagado

//Métodos para encapsular las funcionalidades
//it reads all the sensors
void LeerSensores(void)
{
   //First, we read the photoresistor value
   luminosidad = analogRead(sensorluzpin); 

   //recibe la temperatura para el sensor LM35
   //temperatura = analogRead(temperaturapin);   
   //temperatura = (5.0 * temperatura * 100.0)/1024.0; 

   //Then, we read the temperature sensor on pin 17
   temperatura = (analogRead(temperaturapin) * (3300 / 1024));  
   temperatura = (temperatura - 500) / 10;
 
   //Lee estado de sensor de Temperatura para GROVE temp
   //int B=3975; //Valor del termistor
   //temperatura = analogRead(temperaturapin); //Obtencion del valor leido
   //float resistance=(float)(1023-temperatura)*10000/temperatura; //Obtencion del valor de la resistencia
   //temperatura=1/(log(resistance/10000)/B+1/298.15)-273.15; //Calculo de la temperatura
}

void ImprimirValoresSensores(void)
{
 //it prints the values on the serial monitor
 //We can see the temperature, the light level, and the state of both the fan and the lamp
 //This helps us check if our system is reacting correctly
  Serial.println("========================================");
  
 //Temeratura
 Serial.print("Temperatura: ");
 Serial.print(temperatura);
 Serial.print(" ");
 Serial.println(" ℃ ");

 //Luminosidad
 Serial.print("Luminosidad: ");
 Serial.print(luminosidad);
 Serial.print(" ");
 Serial.println(" V. ");

 //Estado del Bombillo
 if (estadobombillo == false)
    Serial.println("Bombillo Apagado");
 else
    Serial.println("Bombillo Encendido");

 //It compares the current temperature with the threshold we defined
 //If the temperature is higher than the threshold, the fan (red LED) turns on. Otherwise, it turns off

 if (estadoventilador == false)
    Serial.println("Ventilador Apagado");
 else
    Serial.println("Ventilador Encendido");
}

//Similarly, the function UmbraldeLuz() checks the light level
//If the light is below the threshold, it means the room is dark — so the white LED turns on
//If there’s enough light, it stays off
This simulates an automatic room light that turns on at night or in low light.”
boolean UmbraldeTemperatura(float umbral)
{
  if(temperatura > umbral){
    digitalWrite(ventiladorpin, HIGH); 
    delay(1000);
    return true;
  }  
  else{
    digitalWrite(ventiladorpin, LOW);  
    delay(10); 
    return false;
  } 
}

boolean UmbraldeLuz(float umbral)
{
  //Envia una señal que activa o desactiva el relay
  if(luminosidad < umbral){
    digitalWrite(bombillopin, HIGH);
    delay(1000);
    return true;
  }   
  else{
    digitalWrite(bombillopin, LOW);
    delay(10);
    return false;  
  }
}



//metodo cliente para controlar los eventos R1 y R2
void setup()
{
  //Abrir el puerto de lectura en el PC para mensajes
  Serial.begin(115200);

  // Resolución Sensores ADC
  //Resolucion de los puesrtos ADC
  analogReadResolution(12);

  //Establecer los modos de los puertos
  pinMode(sensorluzpin, INPUT);
  pinMode(bombillopin, OUTPUT);
  pinMode(ventiladorpin, OUTPUT);
  pinMode(temperaturapin, INPUT);
  
}

//metodo repetitivo
unsigned long lastConnectionTime = 0;
long lastUpdateTime = 0;

void loop()                    
{

    LeerSensores();
    ImprimirValoresSensores();

    //Verificar los umbrales
    estadoventilador = UmbraldeTemperatura(umbralTemperatura);
    estadobombillo = UmbraldeLuz(umbralLuz);

    //Esperar dos segundos para la nueva medición de
    delay(2000);

}