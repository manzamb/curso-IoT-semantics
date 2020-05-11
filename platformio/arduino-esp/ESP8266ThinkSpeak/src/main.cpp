#include <Arduino.h>
#include <ThingSpeak.h>
#include <ESP8266WiFi.h>
#include <math.h>
#include "rgb_lcd.h"

// Network Parameters
//const char* ssid     = "GtiNetwork";
//const char* password = "gi3z-mig1-tpkz";
const char* ssid = "Angelonet";
const char* password = "Marcus336";

// ThingSpeak information
char thingSpeakAddress[] = "api.thingspeak.com";
unsigned long channelID = 800119;
char* readAPIKey = (char*)"69IY9W03ZXF67X5U";
char* writeAPIKey = (char*)"7ZXS3OPHWQFL6Q1I";
const unsigned long postingInterval = 20L * 1000L;
unsigned int dataFieldOne = 1;                       // Field to write estado bombillo
unsigned int dataFieldTwo = 2;                       // Field to write temperature data
unsigned int dataFieldThree = 3;                       // Field to write tiempo

//RGB Global variable
const int colorR = 0;
const int colorG = 0;
const int colorB = 255;
rgb_lcd lcd;

// Global variables
// These constants are device specific.  You will need to get them from the manufacturer or determine them yourself.
//Parametros de los pines
const int temperaturapin = A0;
const int ledpin = D5;
const int umbraltemperatura = 28;

unsigned long lastConnectionTime = 0;
long lastUpdateTime = 0; 
WiFiClient client;                                  

void connectWiFi(void){
  delay(10);
  // We start by connecting to a WiFi network  
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  //RGB comand
  lcd.setCursor(0, 1);
  lcd.print("Conectando a ");
  lcd.print(ssid);

  WiFi.begin(ssid, password); 
  lcd.setCursor(0, 2); 
  while (WiFi.status() != WL_CONNECTED)  
  {  
   delay(500);  
   Serial.print(".");
   lcd.print(".");
  }  
  Serial.println("");  
  Serial.println("WiFi connected");  
  // Print the IP address  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  lcd.print("");  
  lcd.println("WiFi conectada");  
  // Print the IP address  
  lcd.print("IP address: ");
  lcd.print( WiFi.localIP());

  ThingSpeak.begin( client );
}


// Use this function if you want to write a single field
int writeTSData( long TSChannel, unsigned int TSField, float data ){
  int  writeSuccess = ThingSpeak.writeField( TSChannel, TSField, data, writeAPIKey ); // Write the data to the channel
  if ( writeSuccess ){
    lcd.setCursor(0, 1);
    lcd.print("Send ThinkSpeak");
    Serial.println( String(data) + " written to Thingspeak." );
    }
    
    return writeSuccess;
}

//use this function if you want multiple fields simultaneously
int write2TSData( long TSChannel, unsigned int TSField1, 
                  float field1Data,unsigned int TSField2, long field2Data,
                  unsigned int TSField3, long field3Data ){

  ThingSpeak.setField( TSField1, field1Data );
  ThingSpeak.setField( TSField2, field2Data );
  ThingSpeak.setField( TSField3, field3Data );

  int printSuccess = ThingSpeak.writeFields( TSChannel, writeAPIKey );
  return printSuccess;
}


void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.setRGB(colorR, colorG, colorB);
  lcd.print("Iniciando ..!");

  Serial.begin(9600);
  delay(1500);
  Serial.println("Start");
  // prepare GPIO2
  pinMode(ledpin, OUTPUT);
  digitalWrite(ledpin, LOW);
  //Conectar a la red
  connectWiFi();
}

void loop() {
    // Only update if posting time is exceeded
    if (millis() - lastUpdateTime >=  postingInterval) {
           
        lastUpdateTime = millis();
        int estaBombillo;

        float readValue = analogRead(temperaturapin);
        //En caso de Ser un TMP36
        //float voltage = (readValue / 1024.0) * 3.3; 
        //float temperatura = (voltage - .5) * 100;

        //En caso de conectar una Grove
        const int R0 = 100000;            // R0 = 100k
        const int B = 4275;               // B value of the thermistor
        float voltage = 1023.0/readValue-1.0;
        voltage = R0*voltage;
        float temperatura = 1.0/(log(voltage/R0)/B+1/298.15)-273.15; // convert to temperature via datasheet

        Serial.println("ADC =  " + String( readValue )+ " Temp = "+String( temperatura ));
        lcd.setCursor(0, 2);
        String medida = "Temp = "+ String( temperatura );
        lcd.print(medida);

        //Cambiar estado del bombillo
        if(temperatura > umbraltemperatura){
            estaBombillo = 1;
            digitalWrite(ledpin, HIGH);
        }
        else
        {
          estaBombillo = 0;
          digitalWrite(ledpin, LOW);
        }
        write2TSData( channelID , dataFieldOne , temperatura , 
                      dataFieldTwo , estaBombillo,
                      dataFieldThree, millis());      // Write the temperature.
    }
}



  float readTSData( long TSChannel,unsigned int TSField )
  {
    
  float data =  ThingSpeak.readFloatField( TSChannel, TSField, readAPIKey );
  Serial.println( " Data read from ThingSpeak: " + String( data, 9 ) );
  return data;
  
  }
