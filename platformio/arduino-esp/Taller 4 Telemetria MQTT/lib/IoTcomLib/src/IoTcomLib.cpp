#include <IoTcomLib.h>

//------------------- FUNCIONES PARA CONECTAR A LA RED ---------------------
#include <ESP8266WiFi.h>


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