//Este skecht se ha desarrollado para activar el bombillo a partir del umbral
//también el ventilador cuando la temperatura suba por encima de cierto umbral
//Comunicaciones: Utiliza ThingSpeak para las telecomunicaciones con ESP8266 - Entorno Arduino
//Tambien se ha modificadopar encapsular las funcionalidades añadidas en cada nuevo taller
#include "Arduino.h"
#include <IoTdeviceLib.h>       //Librería con funciones de sensor - actuador
#include <WiFiManager.h>
#include <SPI.h>
#include <Ethernet.h>

//Parametros para la conexi'on a internet
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 211, 177);
EthernetClient client;

char serverC[] = "www.pasted.co";
char dataLocationC[] = "/2434bc64 HTTP / 1.1";



//const int sensorluzpin = A3;  //Fotocelda Grove
const int bombillopin = D3;      //Bombillo
const int ventiladorpin = D5;   //Relay del ventilador
const int temperaturapin = A0;  //Temperatura Grove 
const int luminosidadpin = A6;  //Pin sensor de luminosidad

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
// direcciones IP, servidor, y MAC
// necesarias para Ethernet Shield
IPAddress ip(172, 16, 0, 100);
IPAddress server(172, 16, 0, 2);
byte mac[] = {
    0xDE,
    0xED,
    0xBA,
    0xFE,
    0xFE,
    0xED};

// instanciar objetos
EthernetClient ethClient;
PubSubClient client(ethClient);

// constantes del MQTT
// direccion broker, puerto, y nombre cliente
const char *MQTT_BROKER_ADRESS = "192.168.1.150";
const uint16_t MQTT_PORT = 1883;
const char *MQTT_CLIENT_NAME = "ArduinoClient_1";

// realiza las suscripción a los topic
// en este ejemplo, solo a 'hello/world'
void SuscribeMqtt()
{
    mqttClient.subscribe("hello/world");
}

// callback a ejecutar cuando se recibe un mensaje
// en este ejemplo, muestra por serial el mensaje recibido
void OnMqttReceived(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Received on ");
    Serial.print(topic);
    Serial.print(": ");

    String content = "";
    for (size_t i = 0; i < length; i++)
    {
        content.concat((char)payload[i]);
    }
    Serial.print(content);
    Serial.println();
}

// inicia la comunicacion MQTT
// inicia establece el servidor y el callback al recibir un mensaje
void InitMqtt()
{
    mqttClient.setServer(MQTT_BROKER_ADRESS, MQTT_PORT);
    mqttClient.setCallback(OnMqttReceived);
}

// conecta o reconecta al MQTT
// consigue conectar -> suscribe a topic y publica un mensaje
// no -> espera 5 segundos
void ConnectMqtt()
{
    Serial.print("Starting MQTT connection...");
    if (mqttClient.connect(MQTT_CLIENT_NAME))
    {
        SuscribeMqtt();
        client.publish("connected","hello/world");
    }
    else
    {
        Serial.print("Failed MQTT connection, rc=");
        Serial.print(mqttClient.state());
        Serial.println(" try again in 5 seconds");

        delay(5000);
    }
}

// gestiona la comunicación MQTT
// comprueba que el cliente está conectado
// no -> intenta reconectar
// si -> llama al MQTT loop
void HandleMqtt()
{
    if (!mqttClient.connected())
    {
        ConnectMqtt();
    }
    mqttClient.loop();
}
//metodo cliente para controlar los eventos R1 y R2
void setup()
{
  //Abrir el puerto de lectura en el PC para mensajes
  Serial.begin(115200);

  if (Ethernet.begin(mac) == 0)
   {
      Serial.println("Failed to configure Ethernet using DHCP");
      Ethernet.begin(mac, ip);
   }

   delay(1000);
   Serial.println("connecting...");

   if (client.connect(serverC, 80))
   {
      Serial.println("connected");
      client.print("GET ");
      client.println(dataLocationC);
      client.print("Host: ");
      client.println(serverC);
      client.println();
   }
   else 
   {
      Serial.println("connection failed");
   }
}
  //----------- Fin de conección ESP8266 -----------------------------

  //************* Inicializar Servidor MQTT *********************
   //Inicializar el canal MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  //************* FIN Inicializar Servidor MQTT *****************

  //Establecer los modos de los puertos
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
      //temperatura = LeerTemperatura(temperaturapin, GroveTmp,3.3);
      temperatura = LeerTemperatura(temperaturapin,Tmp36,5.0);
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
      //estadobombillo = UmbralMenorDeSensorActuador(luminosidad,umbralLuz,luminosidadpin);
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
    }
}