//Este skecht se ha desarrollado para activar el bombillo a partir del umbral
//también el ventilador cuando la temperatura suba por encima de cierto umbral
//Comunicaciones: Utiliza ThingSpeak para las telecomunicaciones con ESP8266 - Entorno Arduino
//Tambien se ha modificadopar encapsular las funcionalidades añadidas en cada nuevo taller
#include "Arduino.h"
#include <IoTdeviceLib.h>       //Librería con funciones de sensor - actuador
#include <IoTcomLib.h>          //Librería con funciones de comunicación del dispositivo
#include <DNSServer.h>          //Es necesario instalar la librería EspSoftwareSerial y Wifimanager 
#include <WiFiManager.h>

//-------------------------- Fin Configuración WIFI ESP32 --------------

//Entradas digitales del ESP 32
const int bombillopin = 19;     //Simulado con un led rojo
const int ventiladorpin =18;   //Simulado con un led azul
const int temperaturapin = 26;  //Temperatura DHT11. 

//Entradas Analogas del ESP 32
const int potenciometro = 34;   //Poteciometro para ejemplo PWM
const int sensorluzpin = 39;    //Fotocelda que 

//Variables Globales
int umbralLuz = 1800;                                //Es el umbral en el cual se enciende el bombillo
int umbralTemperatura = 26;                         //Es el umbral en el cual se enciende el ventilador
float luminosidad;                                  //Toma el valor en voltaje
float temperatura;                                  //Toma el valor en grados
boolean estadoventilador =false;                    //false = apagado
boolean estadobombillo = false;                     //false = apagado
int nummedicion = 0;                                //Establece el número consecutivo de observacion hecha
const unsigned long postingInterval = 5L * 1000L;  //Establece cada cuanto se envia a ThingSpeak
unsigned long lastConnectionTime = 0;               //Para controlar el tiempo de generar nueva medición
long lastUpdateTime = 0;                            //Momento de la última actualización

//Variables del Servidor MQTT
int pinOnOff = 5;                                   //Pin para controlar el actuador via MQTT
float sensorValue = 0.0;                            //Valor del sensor para publicar via MQTT 
char mqtt_server[50] ="test.mosquitto.org";         //Dirección del servidor MQTT
//char mqtt_server[50] ="192.168.211.86";             //Dirección del servidor MQTT
char msg[50];                                       //Mensaje a publicar


//metodo cliente para controlar los eventos R1 y R2
void setup()
{
  //Abrir el puerto de lectura en el PC para mensajes
  Serial.begin(115200);

  // Resolución Sensores ADC
  //Resolucion de los puesrtos ADC
  analogReadResolution(10);
  
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
  pinMode(sensorluzpin, INPUT);
  pinMode(bombillopin, OUTPUT);
  pinMode(ventiladorpin, OUTPUT);
  pinMode(temperaturapin, INPUT);

  /*
  //inicializar aqui thingspeak
  //Use mutable char arrays to avoid converting string literal to 'char*'
  static char thingSpeakApiKey[] = "N1GUYGW9Q6G5HKNX";
  static char thingSpeakWriteKey[] = "FOW8AZ8WE21JPEKG";
  unsigned int thingSpeakChannelID = 3094713;
  InicializarThingSpeak(thingSpeakApiKey, thingSpeakWriteKey, thingSpeakChannelID);
  */

   //Inicializar el canal MQTT
   MQTTsetup(mqtt_server);
}

void loop()                    
{
  //Conectarse al servidor MQTT
  MQTTloop();

  //Suscribirse a los topicos de interes MQTT
  SucribirseMQTT((char*)"accionLed");

  // Solamente actualiza si el tiempo de publicación es excedido
  if (millis() - lastUpdateTime >=  postingInterval) {
      lastUpdateTime = millis();

      //LeerSensores
      temperatura = LeerTemperatura(temperaturapin,dht,5.0);
      luminosidad = LeerLuminosidad(sensorluzpin);

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
      estadobombillo = UmbralMenorDeSensorActuador(luminosidad,umbralLuz,bombillopin);
      ImprimirEstadoActuador(bombillopin,"Bobillo Sala");
      Serial.println("========================================");

      //Enviar los Datos a ThinkSpeak
      /*
      EnviarThingSpeakVariosDatos(1 , temperatura , 
                                  2 , estadobombillo,
                                  3 , estadoventilador,
                                  4, luminosidad); 
      */
                                  
      //Enviar los datos al servidor MQTT
      
      //Publicar la temperatura
      snprintf (msg, 75, "%f", temperatura);
      Serial.print("Publicando temperatura en el Servidor MQTT: ");
      Serial.println(msg);
      PublicarMQTTMensaje((char*)"TemperaturaSalida", msg);

      //Publicar el Estado del Ventilador
      snprintf (msg, 75, "%i", estadoventilador);
      Serial.print("Publicando el estado del ventilador en el Servidor MQTT: ");
      Serial.println(msg);
      PublicarMQTTMensaje((char*)"ventiladorSalida", msg);

      //Publicar la luminosidad actual
      Serial.println(luminosidad);
      snprintf (msg, 75, "%f", luminosidad);
      Serial.print("Publicando la luminosidad en el Servidor MQTT: ");
      Serial.println(msg);
      PublicarMQTTMensaje((char*)"luminosidadSalida", msg);

      //Publicar el estado del bombillo
      snprintf (msg, 75, "%i", estadobombillo);
      Serial.print("Publicando el estado del bombillo en el Servidor MQTT: ");
      Serial.println(msg);
      PublicarMQTTMensaje((char*)"bombilloSalida", msg);   

      //Cambiar el estado del actuador via MQTT
      if (isMqttCallback()){
        //Se hace el flag en false para no repetir la acción 
        setCallbackFlag(false); //Resetear el flag
        //Realiza las acciones para todos los topicos suscritos
        switchMQTT((char*)"accionLed", pinOnOff, msg);
      }
  }
}