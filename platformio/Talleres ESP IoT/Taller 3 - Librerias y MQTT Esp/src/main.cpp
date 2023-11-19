//Este skecht se ha desarrollado para activar el bombillo a partir del umbral
//también el ventilador cuando la temperatura suba por encima de cierto umbral
//Comunicaciones: Utiliza ThingSpeak para las telecomunicaciones con ESP8266 - Entorno Arduino
//Tambien se ha modificadopar encapsular las funcionalidades añadidas en cada nuevo taller
#include "Arduino.h"
#include <IoTdeviceLib.h>       //Librería con funciones de sensor - actuador
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>


//const int sensorluzpin = A3;  //Fotocelda Grove
const int bombillopin = LED_BUILTIN;      //Bombillo
const int ventiladorpin = D3;   //Relay del ventilador
const int temperaturapin = A0;  //Temperatura Grove 
const int luminosidadpin = A0;  //Pin sensor de luminosidad

//Variables Globales
int umbralLuz = 500;                                //Es el umbral en el cual se enciende el bombillo
int umbralTemperatura = 27;                         //Es el umbral en el cual se enciende el ventilador
float luminosidad;                                  //Toma el valor en voltaje
float temperatura;                                  //Toma el valor en grados
int estadoventilador =0;                            //0 = apagado
int estadobombillo = 0;                             //0 = apagado
int nummedicion = 0;                                //Establece el número consecutivo de observacion hecha
const unsigned long postingInterval = 5L * 1000L;  //Establece cada cuanto se envia a ThingSpeak
unsigned long lastConnectionTime = 0;               //Para controlar el tiempo de generar nueva medición
long lastUpdateTime = 0;                            //Momento de la última actualización


//************************ Configurar MQTT ************************
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

//Parametros para los mensajes MQTT
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

//Broquer MQTT
//const char* mqtt_server = "iot.eclipse.org";
//Servidor en la ORANGEPi
const char* mqtt_server ="192.168.127.7";
//const char* mqtt_server = "192.168.121.81";

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 0 was received as first character
  if ((char)payload[0] == '0') {
    digitalWrite(bombillopin, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(bombillopin, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      if (temperatura > 0){
        snprintf (msg, 75, "%f", temperatura);
        client.publish("temperaturaSalida",msg);
        Serial.println("enviando...");
        Serial.println(msg);
      }
      // ... and resubscribe
      client.subscribe("accionLed");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
//********************* FIN Configurarción MQTT ************************


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

  //************* Inicializar Servidor MQTT *********************
   //Inicializar el canal MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  //************* FIN Inicializar Servidor MQTT *****************

  //Establecer los modos de los puertos
  //Se comenta este porque este sensor es simulado
  //pinMode(sensorluzpin, INPUT);
  pinMode(bombillopin, OUTPUT);
  pinMode(ventiladorpin, OUTPUT);
  pinMode(temperaturapin, INPUT);
}

void loop()                    
{
  //*********** Conectarse al servidor MQTT ****************
//Intentarconectarse al servidor MQTT
   if (!client.connected()) {
    reconnect();
  }
  client.loop();
  //*********** FIn conectarse al Servidor MQTT ************


  // Solamente actualiza si el tiempo de publicación es excedido
  if (millis() - lastUpdateTime >=  postingInterval) {
      lastUpdateTime = millis();

      //LeerSensores
      temperatura = LeerTemperatura(temperaturapin, GroveTmp,3.3);
      //Para una lectura directa del sensor la linea es la suguiente
      //luminosidad = LeerLuminosidad(luminosidadpin);
      //Para una simulación del sensor se utioliza en mismo dato de temperatura
      luminosidad = LeerLuminosidad(temperatura);

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
      //En caso de querer controlarlo con mqtt comentar la siguiente linea
      estadobombillo = UmbralMenorDeSensorActuador(luminosidad,umbralLuz,bombillopin);
      ImprimirEstadoActuador(bombillopin,"Bobillo Sala");
      Serial.println("========================================"); 

      //Enviar los datos al servidor MQTT
      //Publicar la temperatura
      snprintf (msg, 75, "%f", temperatura);
      Serial.print("Publicando temperatura en el Servidor MQTT: ");
      Serial.println(msg);
      client.publish("temperaturaSalida", msg);

      //Publicar el Estado del Ventilador
      snprintf (msg, 75, "%i", estadoventilador);
      Serial.print("Publicando el estado del ventilador en el Servidor MQTT: ");
      Serial.println(msg);
      client.publish("ventiladorSalida", msg);

      //Publicar la luminosidad actual
      Serial.println(luminosidad);
      snprintf (msg, 75, "%f", luminosidad);
      Serial.print("Publicando la luminosidad en el Servidor MQTT: ");
      Serial.println(msg);
      client.publish("luminosidadSalida", msg);

      //Publicar el estado del bombillo
      snprintf (msg, 75, "%i", estadobombillo);
      Serial.print("Publicando el estado del bombillo en el Servidor MQTT: ");
      Serial.println(msg);
      client.publish("bombilloSalida", msg);
    }
}