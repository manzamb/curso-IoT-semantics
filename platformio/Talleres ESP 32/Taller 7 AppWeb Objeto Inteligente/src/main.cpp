#include <Arduino.h>

#include <IoTdeviceLib.h>       //Librería con funciones de sensor - actuador
#include <IoTcomLib.h>          //Librería con funciones de comunicación del dispositivo
#include <WebServer.h>
//#include <DNSServer.h>          //Es necesario instalar la librería EspSoftwareSerial y Wifimanager 
//#include <WiFiManager.h>         //Librería para configuración WIFI desde celular

//Entradas digitales del ESP 32
const int bombillopin = 19;     //Simulado con un led azul
const int ventiladorpin =18;   //Simulado con un led rojo
const int temperaturapin = 26;  //Temperatura DHT11. 

//Entradas Analogas del ESP 32
const int potenciometro = 34;   //Poteciometro para ejemplo PWM
const int sensorluzpin = 39;    //Fotocelda

//Variables Globales
int umbralLuz = 200;                                //Es el umbral en el cual se enciende el bombillo
int umbralTemperatura = 23;                         //Es el umbral en el cual se enciende el ventilador
float luminosidad;                                  //Toma el valor en voltaje
float temperatura;                                  //Toma el valor en grados
boolean estadoventilador =false;                    //false = apagado
boolean estadobombillo = false;                     //false = apagado
int nummedicion = 0;                                //Establece el número consecutivo de observacion hecha
const unsigned long postingInterval = 5L * 1000L;   //Establece cada cuanto se envia a ThingSpeak
unsigned long lastConnectionTime = 0;               //Para controlar el tiempo de generar nueva medición
long lastUpdateTime = 0;                            //Para controlar el tiempo de nueva publicación 

// Variables para establecer el Modo manual o automático con la AppWeb
bool lightManual = false;
bool fanManual = false;

//Conección WiFi
char ssid[] = "sumothings";
char password[] = "sum0th1ngs@manzamb";

// -------- SERVIDOR --------
WebServer server(80);


String getPage() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<meta http-equiv='refresh' content='2'>"; // refresco cada 2s
  html += "<title>Objetos Inteligentes</title>";
  html += "<style>";
  html += "body { font-family: Arial; margin:20px; }";
  html += "h1 { color:#333; }";
  html += ".card { border:1px solid #ccc; padding:15px; margin-bottom:20px; border-radius:8px; }";
  html += "button { padding:8px 15px; margin:5px; }";
  html += ".on { background:#4CAF50; color:white; }";
  html += ".off { background:#f44336; color:white; }";
  html += ".auto { background:#2196F3; color:white; }";
  html += "</style></head><body>";

  html += "<h1>Gateway ESP32 - Taller 5</h1>";

  // ----- SERVICIO 1: LUZ -----
  html += "<div class='card'>";
  html += "<h2>Servicio 1: Regulador de Luz</h2>";
  html += "Valor LDR: " + String(luminosidad) + "<br>";
  html += "Modo: " + String(lightManual ? "MANUAL" : "AUTOMÁTICO") + "<br>";
  html += "Estado Bombillo: " + String(estadobombillo ? "ENCENDIDO" : "APAGADO") + "<br><br>";

  html += "<form action='/light/on' method='GET' style='display:inline;'>";
  html += "<button class='on' type='submit'>Encender</button></form>";

  html += "<form action='/light/off' method='GET' style='display:inline;'>";
  html += "<button class='off' type='submit'>Apagar</button></form>";

  html += "<form action='/light/auto' method='GET' style='display:inline;'>";
  html += "<button class='auto' type='submit'>Automático</button></form>";
  html += "</div>";

  // ----- SERVICIO 2: TEMPERATURA -----
  html += "<div class='card'>";
  html += "<h2>Servicio 2: Regulador de Temperatura</h2>";
  html += "Temperatura: " + String(temperatura) + " °C<br>";
  html += "Modo: " + String(fanManual ? "MANUAL" : "AUTOMÁTICO") + "<br>";
  html += "Estado Ventilador: " + String(estadoventilador ? "ENCENDIDO" : "APAGADO") + "<br><br>";

  html += "<form action='/fan/on' method='GET' style='display:inline;'>";
  html += "<button class='on' type='submit'>Encender</button></form>";

  html += "<form action='/fan/off' method='GET' style='display:inline;'>";
  html += "<button class='off' type='submit'>Apagar</button></form>";

  html += "<form action='/fan/auto' method='GET' style='display:inline;'>";
  html += "<button class='auto' type='submit'>Automático</button></form>";
  html += "</div>";

  html += "</body></html>";
  return html;
}

// -------- MANEJADORES --------
void handleRoot() {
  server.send(200, "text/html", getPage());
}

void handleLightOn() {
  lightManual = true;
  estadobombillo = true;
  digitalWrite(bombillopin, HIGH);
  server.send(200, "text/html", getPage());
}

void handleLightOff() {
  lightManual = true;
  estadobombillo = false;
  digitalWrite(bombillopin, LOW);
  server.send(200, "text/html", getPage());
}

void handleLightAuto() {
  lightManual = false; // vuelve a automático
  server.send(200, "text/html", getPage());
}

void handleFanOn() {
  fanManual = true;
  estadoventilador = true;
  digitalWrite(ventiladorpin, HIGH);
  server.send(200, "text/html", getPage());
}

void handleFanOff() {
  fanManual = true;
  estadoventilador = false;
  digitalWrite(ventiladorpin, LOW);
  server.send(200, "text/html", getPage());
}

void handleFanAuto() {
  fanManual = false; // vuelve a automático
  server.send(200, "text/html", getPage());
}

void handleNotFound() {
  String uri = server.uri();
  Serial.print("⚠️ NotFound URI: ");
  Serial.println(uri);

  String message = "Ruta no encontrada\n\n";
  message += "URI: ";
  message += uri;
  message += "\n";
  server.send(404, "text/plain", message);
}


//----Programa principal----

void setup() {
  //Abrir el puerto de lectura en el PC para mensajes
  Serial.begin(115200);

  // Resolución Sensores ADC
  //Resolucion de los puesrtos ADC
  //analogReadResolution(12); // Resolución de 12 bits (0-4095)
  //analogSetPinAttenuation(sensorluzpin, ADC_11db); // Rango ~0-3.3V
  
   //----------- Comando para Conectarse a la WIFI el ESP ---------
  ConectarRed(ssid,password);  //Conectar con datos desde el programa

  //----------- Fin de conección ESP8266 -----------------------------

//Establecer los modos de los puertos
  pinMode(sensorluzpin, INPUT);
  pinMode(bombillopin, OUTPUT);
  pinMode(ventiladorpin, OUTPUT);
  pinMode(temperaturapin, INPUT);

// ---- RUTAS DEL SERVIDOR ----
  server.on("/", handleRoot);

  server.on("/light/on", handleLightOn);
  server.on("/light/off", handleLightOff);
  server.on("/light/auto", handleLightAuto);

  server.on("/fan/on", HTTP_GET,handleFanOn);
  server.on("/fan/off", handleFanOff);
  server.on("/fan/auto", handleFanAuto);

// Handlers para errores
server.on("/favicon.ico", []() {
  server.send(204); // 204 = No Content
});
  server.onNotFound(handleNotFound); 

  //Iniciar el Servidor Web
  server.begin();
  Serial.println("Servidor web iniciado");
}

void loop() {
  // Manejo del servidor web: Atiende peticiones, envia respuestas
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
      if (!fanManual) {
        estadoventilador = UmbralMayorDeSensorActuador(temperatura,umbralTemperatura,ventiladorpin);
      }
      ImprimirEstadoActuador(ventiladorpin,"Ventilador Sala");
      ImprimirValorSensor(luminosidad,"Luminosidad Sala"," V. ");
      //Se verifica umbral antes de imprimier el estado del actuador
      if (!lightManual) {
        estadobombillo = UmbralMenorDeSensorActuador(luminosidad,umbralLuz,bombillopin);
      }
      ImprimirEstadoActuador(bombillopin,"Bobillo Sala");
      Serial.println("========================================");
  }
}
