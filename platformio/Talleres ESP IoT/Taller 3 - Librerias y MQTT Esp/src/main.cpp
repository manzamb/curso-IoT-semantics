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
const int bombillopin = D5;      //Bombillo
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
//Pins para los tópicos MQTT
int pinUp = bombillopin;                            // Topico para el bombillo: up
int pinCircle = ventiladorpin;                      // Topico para el ventilador: circle
// Automatico o Manual (AppWeb)
bool lightManual = false;
bool fanManual = false;

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
//const char* mqtt_server ="192.168.127.14";
const char* mqtt_server = "broker.hivemq.com";
//const char* mqtt_server = "192.168.121.81";

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("] ");

  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  String t = String(topic);

  // Actuadores: se encienden/apagan segun el payload
  if (t == "up" || t == "circle") {
    int selectedPin = (t == "up") ? pinUp : pinCircle;
    bool turnOn = (message == "1");  // Mensaje "1" => activar
    digitalWrite(selectedPin, turnOn ? HIGH : LOW);
    Serial.print((t == "up") ? "Bombillo" : "Ventilador");
    Serial.println(turnOn ? " ON" : " OFF");

    if (t == "up") {
      lightManual = true;
      estadobombillo = turnOn;
    } else {
      fanManual = true;
      estadoventilador = turnOn;
    }
    return;
  }

  // Ajuste de umbrales: subir o bajar segun el payload
  if (t == "down") {
    int delta = 5;
    umbralTemperatura += (message == "0") ? -delta : delta;
    Serial.print("Nuevo umbral de temperatura: ");
    Serial.println(umbralTemperatura);
    fanManual = false;  // volver a automatico al cambiar umbral
    // aplicar logica automatica inmediatamente
    estadoventilador = UmbralMayorDeSensorActuador(temperatura, umbralTemperatura, ventiladorpin);
    return;
  }

  if (t == "closefar") {
    int delta = 50;
    umbralLuz += (message == "0") ? -delta : delta;
    Serial.print("Nuevo umbral de luz: ");
    Serial.println(umbralLuz);
    lightManual = false;  // volver a automatico al cambiar umbral
    // aplicar logica automatica inmediatamente
    estadobombillo = UmbralMenorDeSensorActuador(luminosidad, umbralLuz, bombillopin);
    return;
  }

  
  if (t == "automatic") {
    lightManual = false;  // volver a automatico
    fanManual = false;    // volver a automatico
    Serial.println("Modo automatico activado para bombillo y ventilador");
    // aplicar logica automatica inmediatamente
    estadoventilador = UmbralMayorDeSensorActuador(temperatura, umbralTemperatura, ventiladorpin);
    estadobombillo = UmbralMenorDeSensorActuador(luminosidad, umbralLuz, bombillopin);
    return;
  }
  Serial.println("Topico no reconocido");
}

void reconnect() {
  // Loop hasta reconectar
  while (!client.connected()) {
    Serial.print("Intentando conexion MQTT...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("conectado");
      if (temperatura > 0) {
        snprintf(msg, 75, "%f", temperatura);
        client.publish("temperaturaSalida", msg);
        Serial.println("enviando...");
        Serial.println(msg);
      }
      // Suscripcion a topicos
      client.subscribe("accionLed");
      client.subscribe("up");
      client.subscribe("circle");
      client.subscribe("down");
      client.subscribe("closefar");
      client.subscribe("automatic");
      Serial.println("Suscripciones completadas");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
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
      //temperatura = 25;
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
          // Control ventilador
      if (!fanManual) {
        estadoventilador = UmbralMayorDeSensorActuador(temperatura, umbralTemperatura, ventiladorpin);
      } else {
        estadoventilador = digitalRead(ventiladorpin);
      }
      ImprimirEstadoActuador(ventiladorpin,"Ventilador Sala");
      ImprimirValorSensor(luminosidad,"Luminosidad Sala"," V. ");
      //Se verifica umbral antes de imprimier el estado del actuador
      // Control bombillo
      if (!lightManual) {
        estadobombillo = UmbralMenorDeSensorActuador(luminosidad, umbralLuz, bombillopin);
      }  else {
      estadobombillo = digitalRead(bombillopin);
      }
      ImprimirEstadoActuador(bombillopin,"Bobillo Sala");

      Serial.println("=============== UMBRAlES =========================");
      Serial.print("Umbral luz: ");
      Serial.print(umbralLuz);
      snprintf (msg, 75, "%d", umbralLuz);
      client.publish("UmbralActualLuz", msg);

      Serial.print(" | Umbral temp: ");
      Serial.println(umbralTemperatura);
      snprintf (msg, 75, "%d", umbralTemperatura);
      client.publish("UmbralActualTemperatura", msg);

      Serial.print("Modo bombillo: ");
      Serial.println(lightManual ? "Manual" : "Automatico");
      Serial.print("Modo ventilador: ");
      Serial.println(fanManual ? "Manual" : "Automatico");
      Serial.println("=============== PUBLICANDO MQTT =========================");

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
      Serial.println("=====================================================");
    }
}