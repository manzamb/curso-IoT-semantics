//Este skecht se ha desarrollado para activar el bombillo a partir del umbral
//también el ventilador cuando la temperatura suba por encima de cierto umbral
#include "Arduino.h"
//*************** Coneción a ThinkSpeak *********
#include <ThingSpeak.h>

// Información del Canal y Campos de ThingSpeak
char thingSpeakAddress[] = "api.thingspeak.com";
unsigned long channelID = 2477315;
char* readAPIKey = (char*)"SXDNVOX2SOAJ22TN";
char* writeAPIKey = (char*)"HYCWZ40XK7KR4905";
const unsigned long postingInterval = 20L * 1000L;
unsigned int dataFieldOne = 1;                       // Calpo para escribir el estado de la temperatura
unsigned int dataFieldTwo = 2;                       // Campo para escribir el estado del ventilador
unsigned int dataFieldThree = 3;                     // Campo para escribir el estado del bombillo
unsigned int dataFieldFour = 4;                      // FCampo para enviar el tiempo de luz
//*************** Fin Conección ThinkSpeak *******

//------------------------- Activar WIFI ESP8266 -----------------------
#include <ESP8266WiFi.h>

char ssid[] = "sumothings";
char password[] = "sum0th1ngs@manzamb";
WiFiClient client;              //Cliente Wifi para ThingSpeak
//-------------------------- Fin Configuración WIFI ESP8266 --------------

//const int sensorluzpin = A3;    //Fotocelda Grove
const int bombillopin = D5;      //Simulado con un led 13 en Arduino
const int ventiladorpin = D3;    //Relay del ventilador
const int temperaturapin = A0;  //Temperatura Grove 

//Variables Globales
int umbralLuz = 500;            //Es el umbral en el cual se enciende el bombillo
int umbralTemperatura = 27;     //Es el umbral en el cual se enciende el ventilador
float luminosidad;              //Toma el valor en voltaje
float temperatura;              //Toma el valor en grados
boolean estadoventilador=false; //false = apagado
boolean estadobombillo = false; //false = apagado

//Métodos para encapsular las funcionalidades
//Simular lectura de fotocelda 
long fotoceldafuncion()
{
  return random(1023);
}

//Funcion para obtener los valores de los sensores
void LeerSensores(void)
{
   //leer el sensor de luz
   //luminosidad = analogRead(sensorluzpin); 
   luminosidad = fotoceldafuncion();

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
    delay(10);
    return true;
  }   
  else{
    digitalWrite(bombillopin, LOW);
    delay(10);
    return false;  
  }
}

// Use this function if you want to write a single field
int writeTSData( long TSChannel, unsigned int TSField, float data ){
  int  writeSuccess = ThingSpeak.writeField( TSChannel, TSField, data, writeAPIKey ); // Write the data to the channel
  if ( writeSuccess ){
    //lcd.setCursor(0, 1);
    //lcd.print("Send ThinkSpeak");
    Serial.println( String(data) + " written to Thingspeak." );
    }
    
    return writeSuccess;
}

//use this function if you want multiple fields simultaneously
int write2TSData( long TSChannel, unsigned int TSField1, 
                  float field1Data,unsigned int TSField2, long field2Data,
                  unsigned int TSField3, long field3Data ,
                  unsigned int TSField4, long field4Data ){

  ThingSpeak.setField( TSField1, field1Data );
  ThingSpeak.setField( TSField2, field2Data );
  ThingSpeak.setField( TSField3, field3Data );
  ThingSpeak.setField( TSField4, field4Data );

  int printSuccess = ThingSpeak.writeFields( TSChannel, writeAPIKey );
  return printSuccess;
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

  //************ Conectar Cliente ThinkSpeak *******
    ThingSpeak.begin( client );
  //************ Fin Conectar Cliente ThingSpeak ***
  
  //Inicializar el generador de numeros aleatorios
  randomSeed(analogRead(0));
}

//metodo repetitivo
unsigned long lastConnectionTime = 0;
long lastUpdateTime = 0;

void loop()                    
{
  // Only update if posting time is exceeded
  if (millis() - lastUpdateTime >=  postingInterval) {
    lastUpdateTime = millis();
    LeerSensores();
    ImprimirValoresSensores();

    //Verificar los umbrales
    estadoventilador = UmbraldeTemperatura(umbralTemperatura);
    estadobombillo = UmbraldeLuz(umbralLuz);

    //Enviar los Datos a ThinkSpeak
    write2TSData( channelID , dataFieldOne , temperatura , 
                      dataFieldTwo , luminosidad,
                      dataFieldThree , estadoventilador,
                      dataFieldFour , estadobombillo);     
    }
}