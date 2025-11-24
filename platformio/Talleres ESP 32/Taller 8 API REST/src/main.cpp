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
//char ssid[] = "sumothings";
//char password[] = "sum0th1ngs@manzamb";
char ssid[] = "ROXANA";
char password[] = "Cata031318";

// -------- SERVIDOR --------
WebServer server(80);

// Forward declaration para usar la función antes de definirla
String getStatusJson();

String getPage() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<title>Objetos Inteligentes</title>";
  html += "<style>";
  html += "body { font-family: Arial; margin:20px; }";
  html += "h1 { color:#333; }";
  html += ".card { border:1px solid #ccc; padding:15px; margin-bottom:20px; border-radius:8px; }";
  html += "button { padding:8px 15px; margin:5px; cursor:pointer; }";
  html += ".on { background:#4CAF50; color:white; }";
  html += ".off { background:#f44336; color:white; }";
  html += ".auto { background:#2196F3; color:white; }";
  html += ".status { margin:4px 0; }";
  html += "</style></head><body>";

  html += "<h1>Gateway ESP32 - Taller 8</h1>";

  // ----- SERVICIO 1: LUZ -----
  html += "<div class='card'>";
  html += "<h2>Servicio 1: Regulador de Luz</h2>";
  html += "<div class='status'><strong>Valor LDR:</strong> <span id='ldrValue'>--</span></div>";
  html += "<div class='status'><strong>Modo:</strong> <span id='lightMode'>--</span></div>";
  html += "<div class='status'><strong>Estado Bombillo:</strong> <span id='lightState'>--</span></div>";
  html += "<div class='status'><strong>Umbral:</strong> <span id='lightThresholdText'>--</span></div>";
  html += "<div class='status'><input type='number' id='lightThresholdInput' style='width:80px;'/> <button data-th-action='light'>Guardar Umbral</button></div>";
  html += "<button class='on' data-action='light/on'>Encender</button>";
  html += "<button class='off' data-action='light/off'>Apagar</button>";
  html += "<button class='auto' data-action='light/auto'>Automático</button>";
  html += "</div>";

  // ----- SERVICIO 2: TEMPERATURA -----
  html += "<div class='card'>";
  html += "<h2>Servicio 2: Regulador de Temperatura</h2>";
  html += "<div class='status'><strong>Temperatura:</strong> <span id='tempValue'>--</span> °C</div>";
  html += "<div class='status'><strong>Modo:</strong> <span id='fanMode'>--</span></div>";
  html += "<div class='status'><strong>Estado Ventilador:</strong> <span id='fanState'>--</span></div>";
  html += "<div class='status'><strong>Umbral:</strong> <span id='fanThresholdText'>--</span></div>";
  html += "<div class='status'><input type='number' id='fanThresholdInput' style='width:80px;'/> <button data-th-action='fan'>Guardar Umbral</button></div>";
  html += "<button class='on' data-action='fan/on'>Encender</button>";
  html += "<button class='off' data-action='fan/off'>Apagar</button>";
  html += "<button class='auto' data-action='fan/auto'>Automático</button>";
  html += "</div>";

  // Estado inicial para que la UI no arranque en blanco
  html += "<script>";
  html += "const apiStatus = '/api/status';";
  html += "const buttons = [];";
  html += "function applyStatus(data){";
  html += "  if(!data||!data.light||!data.fan)return;";
  html += "  document.getElementById('ldrValue').textContent = Number(data.light.luminosity).toFixed(2);";
  html += "  document.getElementById('lightMode').textContent = data.light.mode === 'MANUAL' ? 'MANUAL' : 'AUTOMÁTICO';";
  html += "  document.getElementById('lightState').textContent = data.light.isOn ? 'ENCENDIDO' : 'APAGADO';";
  html += "  if(document.getElementById('lightThresholdText')) document.getElementById('lightThresholdText').textContent = data.light.threshold;";
  html += "  if(document.getElementById('lightThresholdInput')) document.getElementById('lightThresholdInput').placeholder = data.light.threshold;";
  html += "  document.getElementById('tempValue').textContent = Number(data.fan.temperature).toFixed(2);";
  html += "  document.getElementById('fanMode').textContent = data.fan.mode === 'MANUAL' ? 'MANUAL' : 'AUTOMÁTICO';";
  html += "  document.getElementById('fanState').textContent = data.fan.isOn ? 'ENCENDIDO' : 'APAGADO';";
  html += "  if(document.getElementById('fanThresholdText')) document.getElementById('fanThresholdText').textContent = data.fan.threshold;";
  html += "  if(document.getElementById('fanThresholdInput')) document.getElementById('fanThresholdInput').placeholder = data.fan.threshold;";
  html += "}";
  html += "function refreshStatus(){";
  html += "  fetch(apiStatus).then(r=>r.json()).then(applyStatus).catch(err=>console.log('Error status',err));";
  html += "}";
  html += "function sendAction(path){";
  html += "  fetch('/api/' + path).then(r=>r.json()).then(applyStatus).catch(err=>console.log('Error action',err));";
  html += "}";
  html += "function updateThreshold(type){";
  html += "  const input = document.getElementById(type + 'ThresholdInput');";
  html += "  if(!input) return;";
  html += "  const value = input.value;";
  html += "  if(value === '') return;";
  html += "  fetch('/api/' + type + '/threshold?value=' + encodeURIComponent(value))";
  html += "    .then(r=>r.json())";
  html += "    .then(data => {";
  html += "       applyStatus(data);";
  html += "       input.value = '';";
  html += "    })";
  html += "    .catch(err=>console.log('Error updating threshold', err));";
  html += "}";
  html += "document.addEventListener('DOMContentLoaded',()=>{";
  html += "  document.querySelectorAll('[data-action]').forEach(btn=>{";
  html += "    btn.addEventListener('click',()=>sendAction(btn.dataset.action));";
  html += "    buttons.push(btn);";
  html += "  });";
  html += "  document.querySelectorAll('[data-th-action]').forEach(btn=>{";
  html += "    btn.addEventListener('click',()=>updateThreshold(btn.dataset.thAction));";
  html += "  });";
  html += "  applyStatus(" + getStatusJson() + ");";
  html += "  refreshStatus();";
  html += "  setInterval(refreshStatus,2000);";
  html += "});";
  html += "</script>";

  html += "</body></html>";
  return html;
}

String getStatusJson() {
  String json = "{";

  json += "\"light\":{";
  json += "\"luminosity\":" + String(luminosidad) + ",";
  json += "\"mode\":\"" + String(lightManual ? "MANUAL" : "AUTO") + "\",";
  json += "\"isOn\":"; 
  json += (estadobombillo ? "true" : "false");
  json += ",";
  json += "\"threshold\":" + String(umbralLuz);
  json += "},";

  json += "\"fan\":{";
  json += "\"temperature\":" + String(temperatura) + ",";
  json += "\"mode\":\"" + String(fanManual ? "MANUAL" : "AUTO") + "\",";
  json += "\"isOn\":"; 
  json += (estadoventilador ? "true" : "false");
  json += ",";
  json += "\"threshold\":" + String(umbralTemperatura);
  json += "}";

  json += "}";
  return json;
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

// Handlers para API REST
void handleApiStatus() {
  String json = getStatusJson();
  server.send(200, "application/json", json);
}

// Handlers para controlar luz via API REST
void handleApiLightOn() {
  lightManual = true;
  estadobombillo = true;
  digitalWrite(bombillopin, HIGH);
  server.send(200, "application/json", getStatusJson());
}

void handleApiLightOff() {
  lightManual = true;
  estadobombillo = false;
  digitalWrite(bombillopin, LOW);
  server.send(200, "application/json", getStatusJson());
}

void handleApiLightAuto() {
  lightManual = false;
  server.send(200, "application/json", getStatusJson());
}

// Handlers para controlar ventilador via API REST
void handleApiFanOn() {
  fanManual = true;
  estadoventilador = true;
  digitalWrite(ventiladorpin, HIGH);
  server.send(200, "application/json", getStatusJson());
}

void handleApiFanOff() {
  fanManual = true;
  estadoventilador = false;
  digitalWrite(ventiladorpin, LOW);
  server.send(200, "application/json", getStatusJson());
}

void handleApiFanAuto() {
  fanManual = false;
  server.send(200, "application/json", getStatusJson());
}

// New handlers for threshold update
void handleApiLightThreshold() {
  if(server.hasArg("value")) {
    umbralLuz = server.arg("value").toInt();
  }
  server.send(200, "application/json", getStatusJson());
}

void handleApiFanThreshold() {
  if(server.hasArg("value")) {
    umbralTemperatura = server.arg("value").toInt();
  }
  server.send(200, "application/json", getStatusJson());
}

//----Programa principal----

void setup() {
  //Abrir el puerto de lectura en el PC para mensajes
  Serial.begin(115200);

  // Resolución Sensores ADC
  //Resolucion de los puesrtos ADC
  analogReadResolution(12); // Resolución de 12 bits (0-4095)
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

//Rutas API REST
  server.on("/api/status", HTTP_GET, handleApiStatus);
  server.on("/api/light/on", HTTP_GET, handleApiLightOn);
  server.on("/api/light/off", HTTP_GET, handleApiLightOff);
  server.on("/api/light/auto", HTTP_GET, handleApiLightAuto);
  server.on("/api/light/threshold", HTTP_GET, handleApiLightThreshold);

  server.on("/api/fan/on", HTTP_GET, handleApiFanOn);
  server.on("/api/fan/off", HTTP_GET, handleApiFanOff);
  server.on("/api/fan/auto", HTTP_GET, handleApiFanAuto);
  server.on("/api/fan/threshold", HTTP_GET, handleApiFanThreshold);

  // Handlers para error por el favicon
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
