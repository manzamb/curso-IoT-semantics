#include "IoTLib.h"

//------ SECCION DE LECTURA Y CONVERSION DIGITAL DE SENSORES ---------- 
float LeerTemperatura(int temperaturapin, TipoSensor ts, float tension)
{
   //Lee el voltaje reportado por el sensor
   float temperatura = analogRead(temperaturapin);

   //Variables para un sensor Grove de temperatura
   int B=3975; //Valor del termistor
   float resistance=(float)(1023-temperatura)*10000/temperatura; //Obtencion del valor de la resistencia

    //Asegura el valor correcto de alimentación en dado de ser erroneo
    if (tension != 5.0 || tension != 3.3)
    {
        tension = 5.0;
    }
    
   //Seleccionar el tipo de sensor de temperatura adecuado y
   //obtener su correspondiente valor digital
    switch (ts)
    {
    case Lm35dz:
         //recibe la temperatura para el sensor LM35DZ 
        temperatura = (tension/1024.0) * temperatura * 100;    
    case Tmp36:
        //recibe la temperatura para el sensor TMP36 
        temperatura = (tension/1024.0) * temperatura * 100 -50; 
        break;
    
    case Lm35:
        //recibe la temperatura para el sensor LM35  
        temperatura = (tension * temperatura * 100.0)/1024.0; 
        break;

    case GroveTmp:        
        //Lee estado de sensor de Temperatura para GROVE temp
        temperatura=1/(log(resistance/10000)/B+1/298.15)-273.15; //Calculo de la temperatura
    default:
        break;
    }
    return temperatura;
}

int LeerLuminosidad(int luminosidadPin)
{
    float luminosidad = analogRead(luminosidadPin);
    return luminosidad;
}

//--------------- SECCION DE INTERACCIÓN CON ACTUADORES --------------- 
int LeerEstadoActuador (int actuadorPin)
{
    return digitalRead(actuadorPin);
}

void CambiarEstadoActuador(int actuadorPin, char nombreActuador[])
{
    int estadoActual = digitalRead(actuadorPin);
    if (estadoActual ==1)
    {
        digitalWrite(actuadorPin,0);
    }
    else
    {
        digitalWrite(actuadorPin,1);
    }
    imprimirEstadoActuador(actuadorPin, nombreActuador);
}

//--------------- SECCION DE INTERACCIÓN DE SENSORES-ACTUADORES --------------- 
int UmbralMayorDeSensorActuador (float dato, float umbral, int actuadorPin)
{
    if(dato > umbral){
        digitalWrite(actuadorPin, 1); 
        delay(1000);
        return 1;
    }  
    else{
        digitalWrite(actuadorPin, 0);  
        delay(10); 
        return 0;
    } 
}

int UmbralMenorDeSensorActuador(float dato, float umbral, int actuadorPin)
{
    if(dato < umbral){
        digitalWrite(actuadorPin, 1); 
        delay(1000);
        return 1;
    }  
    else{
        digitalWrite(actuadorPin, 0);  
        delay(10); 
        return 0;
    } 
}

//--------------- SECCION DE INTERACCIÓN M2H --------------- 
void ImprimirValorSensor(float dato, char entidaddeInteres[],char simboloMedida[])
{
    //Imprimir los valores sensados
    Serial.println("========================================");
    
    //Imprimir el dato reportado
    Serial.print(entidaddeInteres);
    Serial.print(": ");
    Serial.print(dato);
    Serial.print(" ");
    Serial.println(simboloMedida);
}

void ImprimirEstadoActuador (int actuadorPin,char nombreActuador[])
{
    if (digitalRead(actuadorPin) == 1)
    {
        Serial.print(nombreActuador);
        Serial.println(" Encendido");
    }
    else
    {
        Serial.print(nombreActuador);
        Serial.println(" Apagado");
    }
}
