//Este skecht se ha desarrollado para activar el bombillo a partir del umbral
//también el ventilador cuando la temperatura suba por encima de cierto umbral
#include "Arduino.h"

//------------------------- Actiovar WIFI ESP8266 -----------------------
#include <ESP8266WiFi.h>

char ssid[] = "Yeye1";
char password[] = "198219500";
//-------------------------- Fin Configuración WIFI ESP8266 --------------

//const int sensorluzpin = A3;    //Fotocelda Grove
const int bombillopin = 3;      //Simulado con un led 13 en Arduino
const int ventiladorpin = D5;    //Relay del ventilador
const int temperaturapin = A0;  //Temperatura Grove 

//Variables Globales
int umbralLuz = 500;            //Es el umbral en el cual se enciende el bombillo
int umbralTemperatura = 31;     //Es el umbral en el cual se enciende el ventilador
float luminosidad;              //Toma el valor en voltaje
float temperatura;              //Toma el valor en grados
boolean estadoventilador=false; //false = apagado
boolean estadobombillo = false; //false = apagado

//Métodos para encapsular las funcionalidades
void LeerSensores(void)
{
   //leer el sensor de luz
   //luminosidad = analogRead(sensorluzpin); 

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

  //----------- Comando para Conectarse a la WIFI el ESP8266 ---------
  Serial.println("Conectandose a la WIFI!");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi conectada");
  Serial.println(WiFi.localIP());
  //----------- Fin de conección ESP8266 -----------------------------

  //Establecer los modos de los puertos
  //pinMode(sensorluzpin, INPUT);
  pinMode(bombillopin, OUTPUT);
  pinMode(ventiladorpin, OUTPUT);
  pinMode(temperaturapin, INPUT);
  
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