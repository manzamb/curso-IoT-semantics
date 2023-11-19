//Este skecht se ha desarrollado para activar el bombillo a partir del umbral
//también el ventilador cuando la temperatura suba por encima de cierto umbral
#include "Arduino.h"
//#include <ChainableLED.h>
 
#define NUM_LEDS  1                   //numero de led a ser conectados

//ChainableLED leds(7,8, NUM_LEDS);  //Inicializa el actuador GRove RGB leds y los conecta a D7 y D8

const int sensorluzpin = A0;    //Fotocelda Grove
const int bombillopin = 3;      //Simulado con un led 13 en Arduino
const int ventiladorpin = 5;    //Relay del ventilador
const int temperaturapin = A0;  //Temperatura Grove 

//Variables Globales
int umbralLuz = 500;            //Es el umbral en el cual se enciende el bombillo
int umbralTemperatura = 22;     //Es el umbral en el cual se enciende el ventilador
float luminosidad;              //Toma el valor en voltaje
float temperatura;              //Toma el valor en grados
boolean estadoventilador=false; //false = apagado
boolean estadobombillo = false; //false = apagado

//Métodos para encapsular las funcionalidades
void LeerSensores(void)
{
   //leer el sensor de luz
   luminosidad = analogRead(sensorluzpin); 

    //recibe la temperatura para el sensor LM35
   //temperatura = analogRead(temperaturapin);   
   //temperatura = (5.0 * temperatura * 100.0)/1024.0; 
 
   //Lee estado de sensor de Temperatura para GROVE temp
   int B=3975; //Valor del termistor
   temperatura = analogRead(temperaturapin); //Obtencion del valor leido
   float resistance=(float)(1023-temperatura)*10000/temperatura; //Obtencion del valor de la resistencia
   temperatura=1/(log(resistance/10000)/B+1/298.15)-273.15; //Calculo de la temperatura
}

void ImprimirValoresSensores(void)
{
 //Imprimir los valores sensados
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

 //Estado del Ventilador
 if (estadoventilador == false)
    Serial.println("Ventilador Apagado");
 else
    Serial.println("Ventilador Encendido");
}

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
    //leds.setColorRGB(0, 255, 0, 0); //coloca el color rojo
    delay(1000);
    return true;
  }   
  else{
    digitalWrite(bombillopin, LOW);
    //leds.setColorRGB(0, 0, 255, 0); //coloca el color verde
    delay(1000);
    return false;  
  }
}

//metodo cliente para controlar los eventos R1 y R2
void setup()
{
  //Abrir el puerto de lectura en el PC para mensajes
  Serial.begin(115200);

  //Establecer los modos de los puertos
  pinMode(sensorluzpin, INPUT);
  pinMode(bombillopin, OUTPUT);
  pinMode(ventiladorpin, OUTPUT);
  pinMode(temperaturapin, INPUT);

  //Estado inicial de actuadores por defecto
  digitalWrite(bombillopin, LOW);
  digitalWrite(ventiladorpin, LOW);
  
}


//metodo repetitivo
void loop()                    
{
  LeerSensores();
  ImprimirValoresSensores();

  //Verificar los umbrales
  estadoventilador = UmbraldeTemperatura(umbralTemperatura);
  estadobombillo = UmbraldeLuz(umbralLuz);

  
//espera en milisegundos para volver a tomar la temperatura 
  delay(2000);
}