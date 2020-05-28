#ifndef _IoTLib
#define _IoTLib
#include <SoftwareSerial.h>
#include <math.h>

typedef enum SensorType {GroveTmp, Lm35, Tmp36, Lm35dz} TipoSensor;
//typedef enum TipoSensor nuevotipo;

/**
 * @brief Funcion genérica que lee un sensor de temperatura de los
 * dispositivos más usados.
 * 
 * @param temperaturapin número entero del pin que captura la señal de temperatura
 * @param ts clase de sensor de temperatura utilizado
 * @param tension alimentación de voltaje al cual esta conectado el sensor Ej. 3.3 o 5.0
 * @return float Retorna el valor digital del sensor de temperatura
 */
float LeerTemperatura(int temperaturapin, TipoSensor ts, float tension);

/**
 * @brief Esta función retorna el valor de voltaje del sensor de luminosidad
 * 
 * @param luminosidadPin valor entero del pin al cual esta conectado el sensor
 * @return int El valor del voltaje retornado
 */
int LeerLuminosidad(int luminosidadPin);

/**
 * @brief Retorna el estado de un actuador: Se maneja encendido = 1, apagado 0
 * 
 * @param actuadorPin valor entero del pin al cual esta conectado el actuador
 * @return int Estado retornado 1 o 0
 */
int LeerEstadoActuador (int actuadorPin);

/**
 * @brief Permite cambiar el estado del actuador de encendido a apagado y viceversa
 * 
 * @param actuadorPin valor entero del pin al cual esta conectado el actuador
 * @param nombreActuador valor de cadena del nombre del actuador para efectos semánticos del ususario
 */
void CambiarEstadoActuador(int actuadorPin, char nombreActuador[]);

/**
 * @brief Compara el valor del dato si es mayor que el Umbral. En caso se ser así 
 * cambia el estado del pin a encendido.
 * 
 * @param dato valor digital del sensor a comparar con el umbral
 * @param umbral Valor por el cual se compara el dato
 * @param actuadorPin valor entero del actuador que se debe encender si el umbral es sobrepasado.
 * @return int retorna el estado en el cual quedo el actuador: 1 o 0 (encendido o apagado)
 */
int UmbralMayorDeSensorActuador (float dato, float umbral, int actuadorPin);

/**
 * @brief Compara el valor del dato si es menor que el Umbral. En caso se ser así 
 * cambia el estado del pin a encendido.
 * 
 * @param dato valor digital del sensor a comparar con el umbral
 * @param umbral Valor por el cual se compara el dato
 * @param actuadorPin valor entero del actuador que se debe encender si el umbral es sobrepasado.
 * @return int retorna el estado en el cual quedo el actuador: 1 o 0 (encendido o apagado)
 */
int UmbralMenorDeSensorActuador(float dato, float umbral, int actuadorPin);

/**
 * @brief Imprime el valor reportado en la salida serial
 * 
 * @param dato Datoa a imprimir
 * @param entidaddeInteres Valor semántico de la entidad medida
 * @param simboloMedida Simbolo de la medicion realizada
 */
void ImprimirValorSensor(float dato, char entidaddeInteres[],char simboloMedida[]);

/**
 * @brief Imprime el estado actual del actuador y coloca Encendido o Apagado
 * 
 * @param actuadorPin Puerto al que esta conectado el actuador
 * @param nombreActuador Nombre semántico del actuador
 */
void ImprimirEstadoActuador (int actuadorPin,char nombreActuador[]);

#endif