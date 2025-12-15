// Este sketch se ha desarrollado para activar el bombillo a partir del umbral
// tambien el ventilador cuando la temperatura suba por encima de cierto umbral
// Comunicaciones: Utiliza ThingSpeak para las telecomunicaciones con ESP8266 - Entorno Arduino
// Tambien se ha modificado para encapsular las funcionalidades anadidas en cada nuevo taller
#include "Arduino.h"
#include <IoTdeviceLib.h>       // Libreria con funciones de sensor - actuador
//#include <IoTcomLib.h>          // Libreria con funciones de comunicacion del dispositivo
#include <DNSServer.h>          // Es necesario instalar la libreria EspSoftwareSerial y Wifimanager
#include <WiFiManager.h>
#include <PubSubClient.h>

// Entradas digitales del ESP 32
const int bombillopin = 19;     // Simulado con un led azul
const int ventiladorpin = 18;   // Simulado con un led rojo
const int temperaturapin = 26;  // Temperatura TMP36

// Entradas Analogas del ESP 32
const int potenciometro = 34;   // Poteciometro para ejemplo PWM
const int sensorluzpin = 39;    // Fotocelda

// Variables Globales
int umbralLuz = 400;                                // Umbral en el cual se enciende el bombillo
int umbralTemperatura = 23;                         // Umbral en el cual se enciende el ventilador
float luminosidad;                                  // Toma el valor en voltaje
float temperatura;                                  // Toma el valor en grados
boolean estadoventilador = false;                   // false = apagado
boolean estadobombillo = false;                     // false = apagado
int nummedicion = 0;                                // Consecutivo de observacion hecha
const unsigned long postingInterval = 5L * 1000L;  // Cada cuanto se envia a ThingSpeak
unsigned long lastConnectionTime = 0;               // Para controlar el tiempo de generar nueva medicion
long lastUpdateTime = 0;                            // Momento de la ultima actualizacion

// Automatico o Manual (AppWeb)
bool lightManual = false;
bool fanManual = false;

// Parametros MQTT
WiFiClient espClient;
PubSubClient client(espClient);
char msg[50];

// Broker MQTT
const char* mqtt_server = "broker.hivemq.com";

int pinUp = bombillopin;            // Topico para el bombillo: up
int pinCircle = ventiladorpin;      // Topico para el ventilador: circle

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
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// Metodo cliente para controlar los eventos R1 y R2
void setup() {
  Serial.begin(115200);

  // Resolucion de los puertos ADC
  analogReadResolution(12);
  analogSetPinAttenuation(sensorluzpin, ADC_11db); // Rango ~0-3.3V

  // Conectar y configurar desde el celular
  WiFiManager wifiManager;
  wifiManager.autoConnect("ESP8266Temp");
 
  Serial.println("Ya estas conectado");

  // Establecer los modos de los puertos
  pinMode(sensorluzpin, INPUT);
  pinMode(bombillopin, OUTPUT);
  pinMode(ventiladorpin, OUTPUT);
  pinMode(temperaturapin, INPUT);

  // Inicializar el canal MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {               
  // Conectarse al servidor MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Solamente actualiza si el tiempo de publicacion es excedido
  if (millis() - lastUpdateTime >= postingInterval) {
    lastUpdateTime = millis();

    // Leer sensores
    temperatura = LeerTemperatura(temperaturapin, dht, 3.3);
    luminosidad = LeerLuminosidad(sensorluzpin);

    // Imprimir valores sensores y actuadores 
    Serial.print("=========== Medicion No.: ");
    Serial.print(nummedicion++);
    Serial.println(" ============");
    ImprimirValorSensor(temperatura, "Temperatura Sala", " C ");

    // Control ventilador
    if (!fanManual) {
      estadoventilador = UmbralMayorDeSensorActuador(temperatura, umbralTemperatura, ventiladorpin);
    } else {
      estadoventilador = digitalRead(ventiladorpin);
    }
    ImprimirEstadoActuador(ventiladorpin, "Ventilador Sala");

    ImprimirValorSensor(luminosidad, "Luminosidad Sala", " V. ");

    // Control bombillo
    if (!lightManual) {
      estadobombillo = UmbralMenorDeSensorActuador(luminosidad, umbralLuz, bombillopin);
    } else {
      estadobombillo = digitalRead(bombillopin);
    }
    ImprimirEstadoActuador(bombillopin, "Bobillo Sala");
    Serial.print("Umbral luz: ");
    Serial.print(umbralLuz);
    Serial.print(" | Umbral temp: ");
    Serial.println(umbralTemperatura);
    Serial.print("Modo bombillo: ");
    Serial.println(lightManual ? "Manual" : "Automatico");
    Serial.print("Modo ventilador: ");
    Serial.println(fanManual ? "Manual" : "Automatico");
    Serial.println("========================================");

    // Enviar los datos al servidor MQTT
    snprintf(msg, 75, "%f", temperatura);
    Serial.print("Publicando temperatura en el Servidor MQTT: ");
    Serial.println(msg);
    client.publish("temperaturaSalida", msg);

    snprintf(msg, 75, "%i", estadoventilador);
    Serial.print("Publicando el estado del ventilador en el Servidor MQTT: ");
    Serial.println(msg);
    client.publish("ventiladorSalida", msg);

    snprintf(msg, 75, "%f", luminosidad);
    Serial.print("Publicando la luminosidad en el Servidor MQTT: ");
    Serial.println(msg);
    client.publish("luminosidadSalida", msg);

    snprintf(msg, 75, "%i", estadobombillo);
    Serial.print("Publicando el estado del bombillo en el Servidor MQTT: ");
    Serial.println(msg);
    client.publish("bombilloSalida", msg);   
  }
}
