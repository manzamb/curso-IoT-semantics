#include <Arduino.h>

#include <IoTdeviceLib.h>       //Librería con funciones de sensor - actuador
#include <IoTcomLib.h>          //Librería con funciones de comunicación del dispositivo
#include <DNSServer.h>          //Es necesario instalar la librería EspSoftwareSerial y Wifimanager 
#include <WiFiManager.h>         //Librería para configuración WIFI desde celular


//Entradas digitales del ESP 32
const int bombillopin = 19;     //Simulado con un led azul
const int ventiladorpin =18;   //Simulado con un led rojo
const int temperaturapin = 26;  //Temperatura DHT11. 

//Entradas Analogas del ESP 32
const int potenciometro = 34;   //Poteciometro para ejemplo PWM
const int sensorluzpin = 39;    //Fotocelda

//Variables Globales
int umbralLuz = 200;                               //Es el umbral en el cual se enciende el bombillo
int umbralTemperatura = 23;                         //Es el umbral en el cual se enciende el ventilador
float luminosidad;                                  //Toma el valor en voltaje
float temperatura;                                  //Toma el valor en grados
boolean estadoventilador =false;                    //false = apagado
boolean estadobombillo = false;                     //false = apagado
int nummedicion = 0;                                //Establece el número consecutivo de observacion hecha
const unsigned long postingInterval = 5L * 1000L;  //Establece cada cuanto se envia a ThingSpeak
unsigned long lastConnectionTime = 0;               //Para controlar el tiempo de generar nueva medición
long lastUpdateTime = 0; 

// -------- UMBRALES --------
int lightThreshold = 2000;  // Ajustable
float tempThreshold = 28.0; // Ajustable

// -------- SERVIDOR --------
WebServer server(80);


String getPage() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta http-equiv='refresh' content='2'>";
  html += "<title>Objetos Inteligentes</title></head><body>";
  html += "<h1>Gateway ESP32 - Taller 5</h1>";

  html += "<h2>Servicio 1: Regulador de Luz</h2>";
  html += "Valor LDR: " + String(luminosidad) + "<br>";
  html += "Estado Bombillo: " + String(estadobombillo ? "ENCENDIDO" : "APAGADO") + "<br><br>";

  html += "<h2>Servicio 2: Regulador de Temperatura</h2>";
  html += "Temperatura: " + String(temperatura) + "°C<br>";
  html += "Estado Ventilador: " + String(estadoventilador ? "ENCENDIDO" : "APAGADO") + "<br>";

  html += "</body></html>";
  return html;
}

void setup() {
  //Abrir el puerto de lectura en el PC para mensajes
  Serial.begin(115200);

  // Resolución Sensores ADC
  //Resolucion de los puesrtos ADC
  //analogReadResolution(12); // Resolución de 12 bits (0-4095)
  //analogSetPinAttenuation(sensorluzpin, ADC_11db); // Rango ~0-3.3V
  
  //ConectarRed("sumothings","sum0th1ns@manzamb");  //Conectar con datos desde el programa
  //-----Comando para Conectarse y configurar desde el Celular--------
  // Creamos una instancia de la clase WiFiManager
  WiFiManager wifiManager;

  // Descomentar para resetear configuración - Hacer el ejercicio con el celular
  // todas las veces.
  wifiManager.resetSettings();

  // Creamos AP y portal para configurar desde el Celular
  wifiManager.autoConnect("ESP8266Temp");
 
  //Serial.println("!Ya estás conectado¡");
  //----------- Fin de conección ESP8266 -----------------------------

//Establecer los modos de los puertos
  pinMode(sensorluzpin, INPUT);
  pinMode(bombillopin, OUTPUT);
  pinMode(ventiladorpin, OUTPUT);
  pinMode(temperaturapin, INPUT);

// ---- INICIO SERVIDOR WEB ----
  server.on("/", []() {
    server.send(200, "text/html", getPage());
  });
  server.begin();

  Serial.println("Servidor web iniciado");
}

void loop() {
  server.handleClient();

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
  }
}
