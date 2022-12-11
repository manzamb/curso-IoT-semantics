//Este skecht se ha desarrollado para activar el bombillo a partir del umbral
//también el ventilador cuando la temperatura suba por encima de cierto umbral
//Comunicaciones: Utiliza ThingSpeak para las telecomunicaciones con ESP8266 - Entorno Arduino
//Tambien se ha modificadopar encapsular las funcionalidades añadidas en cada nuevo taller
#include "Arduino.h"
#include <IoTdeviceLib.h>       //Librería con funciones de sensor - actuador
#include <IoTcomLib.h>          //Librería con funciones de comunicación del dispositivo
#include <DNSServer.h>          //Es necesario instalar la librería EspSoftwareSerial y Wifimanager 
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

//const int sensorluzpin = A3;  //Fotocelda Grove
const int bombillopin = LED_BUILTIN;      //Simulado con un led 13 en Arduino
const int ventiladorpin = D3;   //Relay del ventilador
const int temperaturapin = A0;  //Temperatura Grove 
const int luminosidadpin = D6;  //Pin sensor de luminosidad

//Variables Globales
int umbralLuz = 500;                                //Es el umbral en el cual se enciende el bombillo
int umbralTemperatura = 28;                         //Es el umbral en el cual se enciende el ventilador
float luminosidad;                                  //Toma el valor en voltaje
float temperatura;                                  //Toma el valor en grados
boolean estadoventilador =false;                    //false = apagado
boolean estadobombillo = false;                     //false = apagado
int nummedicion = 0;                                //Establece el número consecutivo de observacion hecha
const unsigned long postingInterval = 20L * 1000L;  //Establece cada cuanto se envia a ThingSpeak
unsigned long lastConnectionTime = 0;               //Para controlar el tiempo de generar nueva medición
long lastUpdateTime = 0;                            //Momento de la última actualización

//metodo cliente para controlar los eventos R1 y R2
void setup()
{
  //Abrir el puerto de lectura en el PC para mensajes
  Serial.begin(115200);

  //ConectarRed("Redmi","Marcus336");  //Conectar con datos desde el programa
  //-----Comando para Conectarse y configurar desde el Celular--------
  // Creamos una instancia de la clase WiFiManager
  WiFiManager wifiManager;

  // Descomentar para resetear configuración - Hacer el ejercicio con el celular
  // todas las veces.
  //wifiManager.resetSettings();

  // Creamos AP y portal para configurar desde el Celular
  wifiManager.autoConnect("ESP8266Temp");
 
  Serial.println("!Ya estás conectado¡");
  //----------- Fin de conección ESP8266 -----------------------------

  //Establecer los modos de los puertos
  //pinMode(sensorluzpin, INPUT);
  pinMode(bombillopin, OUTPUT);
  pinMode(ventiladorpin, OUTPUT);
  pinMode(temperaturapin, INPUT);

  //inicializar aqui thingspeak
  InicializarThingSpeak();
}

void loop()                    
{
  // Solamente actualiza si el tiempo de publicación es excedido
  if (millis() - lastUpdateTime >=  postingInterval) {
      lastUpdateTime = millis();

      //LeerSensores
      temperatura = LeerTemperatura(temperaturapin,GroveTmp,3.3);
      luminosidad = LeerLuminosidad(luminosidadpin);

      //Imprimir Valores Sensores y Actuadores 
      Serial.print("=========== Medición No.: ");
      Serial.print(nummedicion++);
      Serial.println(" ============");
      ImprimirValorSensor(temperatura,"Temperatura Sala"," ℃ ");
      //Se verifica umbral antes de imprimier el estado del actuador
      estadoventilador = UmbralMayorDeSensorActuador(temperatura,umbralTemperatura,ventiladorpin);
      ImprimirEstadoActuador(ventiladorpin,"Ventilador Sala");
      ImprimirValorSensor(luminosidad,"Luminosidad Sala"," V. ");
      //Se verifica umbral antes de imprimier el estado del actuador
      estadobombillo = UmbralMenorDeSensorActuador(luminosidad,umbralLuz,luminosidadpin);
      ImprimirEstadoActuador(bombillopin,"Bobillo Sala");
      Serial.println("========================================");

      //Enviar los Datos a ThinkSpeak
      
      EnviarThingSpeakVariosDatos(1 , temperatura , 
                                  2 , estadobombillo,
                                  3 , estadoventilador,
                                  4, millis());     
    }
}