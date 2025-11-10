#include <IoTcomLib.h>
#include <ThingSpeak.h>
#include <WiFi.h>
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
