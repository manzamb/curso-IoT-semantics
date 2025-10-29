#pragma once
#include <Arduino.h>
#ifndef _IoTcomLib
#define _IoTcomLib

//------------------- FUNCIONES PARA CONECTAR A LA RED ---------------------
/**
 * @brief Permite conectar el ESP a la Red, dependiendo de los parámetros
 * 
 * @param ssid identificador de la red
 * @param password contraseña de red
 */
void ConectarRed(char ssid[], char password[]);


//------------------- FUNCIONES PARA CONECTAR A THINGSPEAK ---------------------
/**
 * @brief Inicializa el cliente de ThinkSpeak y almacena los datos de conexión
 * 
 * @param TSChannel Id del canal en ThingSpeak
 * @param readAPIKey API KEY de ThingSpeak para solo lectura
 * @param writeAPIKey API KEY de ThinkSpeak para escritura
 * @return void 
 */
void InicializarThingSpeak(char* readAPIKey, char* writeAPIKey, unsigned long TSChannel);

/**
 * @brief Permite enviar un dato perticular a ThingSpeak
 * 
 * @param TSField Identificador del Campo a asignar el Dato
 * @param data Dato a ser enviado a ThingSpeak
 * @return int 
 */
int EnviarThingSpeakDatos(unsigned int TSField, float data );

/**
 * @brief Permite enviar cuatro datos al tiempo. Se puede expandir para enviar mas datos
 * 
 * @param TSField1 Identificador del Campo 1
 * @param field1Data Datos del Campo 1
 * @param TSField2 Identificador del Campo 2
 * @param field2Data Datos del Campo 2
 * @param TSField3 Identificador del Campo 3
 * @param field3Data Datos del Campo 3
 * @param TSField4 Identificador del Campo 4
 * @param field4Data Datos del Campo 4
 * @return int 
 */
int EnviarThingSpeakVariosDatos( unsigned int TSField1, 
                  float field1Data,unsigned int TSField2, long field2Data,
                  unsigned int TSField3, long field3Data ,
                  unsigned int TSField4, long field4Data );

////------------------- FUNCIONES PARA CONECTAR A SERVIDOR MQTT ---------------------
void MQTTsetup(char* mqtt_server_address);
void MQTTloop();
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();
void PublicarMQTTMensaje(char* topic, char* message);
void SucribirseMQTT(char* topic);
void switchMQTT(char* topic, int pinOnOff, char* payload);
boolean isMqttCallback();
void setCallbackFlag(boolean flag);
#endif