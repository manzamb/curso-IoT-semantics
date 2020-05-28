//Este skecht se ha desarrollado para activar el bombillo a partir del umbral
//también el ventilador cuando la temperatura suba por encima de cierto umbral
//Comunicaciones: Utiliza ThingSpeak para las telecomunicaciones con ESP8266 - Entorno Arduino
//Tambien se ha modificadopar encapsular las funcionalidades añadidas en cada nuevo taller
#include "Arduino.h"
#include "IoTLib.h"

//*************** Coneción a ThinkSpeak *********
#include <ThingSpeak.h>

// Información del Canal y Campos de ThingSpeak
char thingSpeakAddress[] = "api.thingspeak.com";
unsigned long channelID = 799494;
char* readAPIKey = (char*)"70GGTLNT0EMFP0WO";
char* writeAPIKey = (char*)"7ZBZ9LU15LQRYKRF";
const unsigned long postingInterval = 20L * 1000L;
unsigned int dataFieldOne = 1;                       // Calpo para escribir el estado de la Temperatura
unsigned int dataFieldTwo = 2;                       // Campo para escribir el estado del Bombillo
unsigned int dataFieldThree = 3;                     // Campo para escribir el estado del ventilador
unsigned int dataFieldFour = 4;                      // FCampo para enviar el tiempo de medición
//*************** Fin Conección ThinkSpeak *******

//------------------------- Activar WIFI ESP8266 -----------------------
#include <ESP8266WiFi.h>

char ssid[] = "Redmi";
char password[] = "Marcus336";
WiFiClient client;              //Cliente Wifi para ThingSpeak
//-------------------------- Fin Configuración WIFI ESP8266 --------------

//const int sensorluzpin = A3;    //Fotocelda Grove
const int bombillopin = 3;      //Simulado con un led 13 en Arduino
const int ventiladorpin = D5;    //Relay del ventilador
const int temperaturapin = A0;  //Temperatura Grove 

//Variables Globales
int umbralLuz = 500;            //Es el umbral en el cual se enciende el bombillo
int umbralTemperatura = 32;     //Es el umbral en el cual se enciende el ventilador
float luminosidad;              //Toma el valor en voltaje
float temperatura;              //Toma el valor en grados
boolean estadoventilador =false; //false = apagado
boolean estadobombillo = false; //false = apagado

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
  
}

//metodo repetitivo
unsigned long lastConnectionTime = 0;
long lastUpdateTime = 0;

void loop()                    
{
  // Only update if posting time is exceeded
  if (millis() - lastUpdateTime >=  postingInterval) {
    lastUpdateTime = millis();

    //LeerSensores();
    LeerTemperatura(temperaturapin, GroveTmp,3.3);
    //ImprimirValoresSensores();
    ImprimirValoresSensores();

    //Verificar los umbrales
    estadoventilador = UmbraldeTemperatura(umbralTemperatura);
    estadobombillo = UmbraldeLuz(umbralLuz);

    //Enviar los Datos a ThinkSpeak
    write2TSData( channelID , dataFieldOne , temperatura , 
                      dataFieldTwo , estadobombillo,
                      dataFieldThree , estadoventilador,
                      dataFieldFour, millis());     
    }
}