#include <IoTcomLib.h>
#include <ThingSpeak.h>
#include <WiFi.h>
#include <PubSubClient.h>

//------------------- FUNCIONES PARA CONECTAR A LA RED ---------------------

void ConectarRed(char ssid[], char password[])
{
      Serial.println("Conectandose a la WIFI!");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi conectada");
  Serial.println(WiFi.localIP());
}

//------------------- FUNCIONES PARA CONECTAR A THINGSPEAK ---------------------
#include <ThingSpeak.h>
WiFiClient client;              //Cliente Wifi para ThingSpeak
// Información del Canal y Campos de ThingSpeak
//static char thingSpeakAddress[] = "api.thingspeak.com";
unsigned long GchannelID;
char* GreadAPIKey;
char* GwriteAPIKey;
unsigned int dataFieldOne = 1;                       // Calpo para escribir el estado de la Temperatura
unsigned int dataFieldTwo = 2;                       // Campo para escribir el estado del Bombillo
unsigned int dataFieldThree = 3;                     // Campo para escribir el estado del ventilador
unsigned int dataFieldFour = 4;                      // Campo para enviar el tiempo de medición

void InicializarThingSpeak(char* readAPIKey, char* writeAPIKey, unsigned long TSChannel)
{
    //************ Conectar Cliente ThinkSpeak *******
    ThingSpeak.begin( client );
    //************ Fin Conectar Cliente ThingSpeak ***
    GreadAPIKey = readAPIKey;
    GchannelID = TSChannel;
    GwriteAPIKey = writeAPIKey;
}

// Use this function if you want to write a single field
int EnviarThingSpeakDatos(unsigned int TSField, float data ){
  int  writeSuccess = ThingSpeak.writeField( GchannelID, TSField, data, GwriteAPIKey ); // Write the data to the channel
  if ( writeSuccess ){
    //lcd.setCursor(0, 1);
    //lcd.print("Send ThinkSpeak");
    Serial.println( String(data) + " Enviando a ThingSpeak." );
    }
    
    return writeSuccess;
}

//use this function if you want multiple fields simultaneously
int EnviarThingSpeakVariosDatos( 
                  unsigned int TSField1, float field1Data,
                  unsigned int TSField2, long field2Data,
                  unsigned int TSField3, long field3Data ,
                  unsigned int TSField4, long field4Data ){

    ThingSpeak.setField( TSField1, field1Data );
    ThingSpeak.setField( TSField2, field2Data );
    ThingSpeak.setField( TSField3, field3Data );
    ThingSpeak.setField( TSField4, field4Data );

    Serial.print("Enviando Datos ThingSpeak...");
    int printSuccess = ThingSpeak.writeFields( GchannelID, GwriteAPIKey );
    Serial.println("Success: "); Serial.print(printSuccess);
    return printSuccess;
}

////------------------- FUNCIONES PARA CONECTAR A SERVIDOR MQTT ---------------------
//************************ Configurar MQTT ************************
//Parametros para los mensajes MQTT
WiFiClient espClient;
PubSubClient mqttClient(espClient);
boolean mqttCallback = false;
char* topicReceibed;
char* payloadReceibed;

//Configuración del servidor MQTT
//mqtt_server_address: dirección IP del servidor MQTT
void MQTTsetup(char* mqtt_server_address) {
  mqttClient.setServer(mqtt_server_address, 1883);
  mqttClient.setCallback(callback);
}

//Conectandose al servidor MQTT
void MQTTloop() {
  // Loop until we're reconnected
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();
}

//Callback que se ejecuta al recibir un mensaje MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  //Establecer que se ejecuto Callback MQTT
  mqttCallback = true;
  topicReceibed = topic;
  payloadReceibed = (char*)payload;
}

boolean isMqttCallback(){
  return mqttCallback;
}

void setCallbackFlag(boolean flag){
  mqttCallback = flag;
}

//Función para reconectarse al servidor MQTT
void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Intentando conexión MQTT...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str())) {
      // Once connected, publish an announcement
      Serial.println("conectado");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//Publicar mensaje MQTT
void PublicarMQTTMensaje(char* topic, char* message){
    mqttClient.publish(topic, message);
}

//Suscribirse a un topico  MQTT
void SucribirseMQTT(char* topic){
    mqttClient.subscribe(topic);
}

//Funciones para controlar sensores y actuadores via MQTT se pueden agregar aquí
void switchMQTT(char* topic, int pinOnOff, char* payload){
  //Código para controlar sensores y actuadores via MQTT
  // Switch on the LED if an 0 was received as first character
  if ((char)payload[0] == '0') {
    digitalWrite(pinOnOff, LOW);   // Turn the pin on (Note that LOW is the voltage level
    } else {
    digitalWrite(pinOnOff, HIGH);  // Turn the pin off by making the voltage HIGH
  }
} 
//********************* FIN Configurarción MQTT ************************