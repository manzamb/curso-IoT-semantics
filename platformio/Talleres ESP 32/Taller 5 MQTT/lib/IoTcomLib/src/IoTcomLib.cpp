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
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

//Broquer MQTT
const char* mqtt_server = "192.168.211.77";
//const char* mqtt_server ="test.mosquitto.org";

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido [");
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
    Serial.print("Intentando conexión MQTT...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("conectado");
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