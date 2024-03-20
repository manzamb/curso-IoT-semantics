//Este skecht se ha desarrollado para activar el bombillo a partir del umbral
//también el ventilador cuando la temperatura suba por encima de cierto umbral
#include "Arduino.h"
//Para utilizar el ADC Grove se llama esta librería
#include <Wire.h>

//Variables para utilizar el ADC Grove
#define ADDR_ADC121             0x50 // For v1.0 & v1.1, I2C address is 0x55
 
#define V_REF 3.00
 
#define REG_ADDR_RESULT         0x00
#define REG_ADDR_ALERT          0x01
#define REG_ADDR_CONFIG         0x02
#define REG_ADDR_LIMITL         0x03
#define REG_ADDR_LIMITH         0x04
#define REG_ADDR_HYST           0x05
#define REG_ADDR_CONVL          0x06
#define REG_ADDR_CONVH          0x07
 
unsigned int getData;
float analogVal=0;         // convert

//const int sensorluzpin = A3;    //Fotocelda Grove
const int bombillopin = D5;      //Simulado con un led 13 en Arduino
const int ventiladorpin = D3;    //Relay del ventilador
const int temperaturapin = A0;  //Temperatura Grove 

//Variables Globales
float umbralLuz = 0.50;         //Es el umbral en el cual se enciende el bombillo Grove ADC Luz fluctua [0 - 2.01]
int umbralTemperatura = 31;     //Es el umbral en el cual se enciende el ventilador
float luminosidad;              //Toma el valor en voltaje
float temperatura;              //Toma el valor en grados
boolean estadoventilador=false; //false = apagado
boolean estadobombillo = false; //false = apagado

//Métodos para encapsular las funcionalidades
//funciones del GAD Grove
void init_adc();
void read_adc();

//Prototipos de funciones de usuario
long fotoceldafuncion();
void LeerSensores(void);
void ImprimirValoresSensores(void);
boolean UmbraldeTemperatura(float umbral);
boolean UmbraldeLuz(float umbral);


//metodo cliente para controlar los eventos R1 y R2
void setup()
{
  //Abrir el puerto de lectura en el PC para mensajes
  Serial.begin(115200);

  //Inicializar ADC
  Wire.begin();
  init_adc();

  //Establecer los modos de los puertos
  //pinMode(sensorluzpin, INPUT);
  pinMode(bombillopin, OUTPUT);
  pinMode(ventiladorpin, OUTPUT);
  pinMode(temperaturapin, INPUT);
  
  //Inicializar el generador de numeros aleatorios
  randomSeed(analogRead(0));
}

//metodo repetitivo
unsigned long lastConnectionTime = 0;
long lastUpdateTime = 0;

void loop()                    
{

    LeerSensores();
    ImprimirValoresSensores();

    //Verificar los umbrales
    estadoventilador = UmbraldeTemperatura(umbralTemperatura);
    estadobombillo = UmbraldeLuz(umbralLuz);

    //Esperar dos segundos para la nueva medición de
    delay(2000);

}

//Seccion de funciones que utiliza el programa principal

void init_adc()
{
  Wire.beginTransmission(ADDR_ADC121);        // transmit to device
  Wire.write(REG_ADDR_CONFIG);                // Configuration Register
  Wire.write(0x20);
  Wire.endTransmission();  
}
 
void read_adc()     //unsigned int *data
{
 
 
    Wire.beginTransmission(ADDR_ADC121);        // transmit to device
    Wire.write(REG_ADDR_RESULT);                // get result
    Wire.endTransmission();
 
    Wire.requestFrom(ADDR_ADC121, 2);           // request 2byte from device
    delay(1);
    if(Wire.available()<=2)
    {
      getData = (Wire.read()&0x0f)<<8;
      getData |= Wire.read();
    }
    //Serial.print("getData:");
    //Serial.println(getData);
    //delay(5);
    //Serial.print("The analog value is:");
    //Serial.print(getData*V_REF*2/4096);
    luminosidad = getData*V_REF*2/4096; 
    //Serial.println("V");
}

//funciones de usuario
//Simular lectura de fotocelda 
// long fotoceldafuncion()
// {
//   return random(1023);
// }

//Funcion para obtener los valores de los sensores
void LeerSensores(void)
{
   //leer el sensor de luz
   //luminosidad = analogRead(sensorluzpin); 
   //luminosidad = fotoceldafuncion();
   read_adc();

    //recibe la temperatura para el sensor LM35
   //temperatura = analogRead(temperaturapin);   
   //temperatura = (5.0 * temperatura * 100.0)/1024.0; 
 
   //Lee estado de sensor de Temperatura para GROVE temp
   int B=3975; //Valor del termistor
   temperatura = analogRead(temperaturapin); //Obtencion del valor leido
   float resistance=(float)(1023-temperatura)*10000/temperatura; //Obtencion del valor de la resistencia
   temperatura=1/(log(resistance/10000)/B+1/298.15)-273.15; //Calculo de la temperatura
}

void ImprimirValoresSensores(void)
{
 //Imprimir los valores sensados
  Serial.println("========================================");
  
 //Temeratura
 Serial.print("Temperatura: ");
 Serial.print(temperatura);
 Serial.print(" ");
 Serial.println(" ℃ ");

 //Luminosidad
 Serial.print("Luminosidad: ");
 Serial.print(luminosidad);
 Serial.print(" ");
 Serial.println(" V. ");

 //Estado del Bombillo
 if (estadobombillo == false)
    Serial.println("Bombillo Apagado");
 else
    Serial.println("Bombillo Encendido");

 //Estado del Ventilador
 if (estadoventilador == false)
    Serial.println("Ventilador Apagado");
 else
    Serial.println("Ventilador Encendido");
}

boolean UmbraldeTemperatura(float umbral)
{
  if(temperatura > umbral){
    digitalWrite(ventiladorpin, HIGH); 
    delay(1000);
    return true;
  }  
  else{
    digitalWrite(ventiladorpin, LOW);  
    delay(10); 
    return false;
  } 
}

boolean UmbraldeLuz(float umbral)
{
  //Envia una señal que activa o desactiva el relay
  if(luminosidad < umbral){
    digitalWrite(bombillopin, HIGH);
    delay(1000);
    return true;
  }   
  else{
    digitalWrite(bombillopin, LOW);
    delay(10);
    return false;  
  }
}
