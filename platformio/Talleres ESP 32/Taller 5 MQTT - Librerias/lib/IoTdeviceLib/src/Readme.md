Librería IoTdeviceLib
Esta libreria encapsula la información del dispositivo y algunos métodos para leer sensores y actuadores, así como métodos para la interacción entre los mismos.

Para usarla: 
- Se copian los archivos en la carpeta /lib/
- instalar la librería ESPSoftwareSerial con Platformio en la etiqueta de librerías
- En el platformio.ini del proyecto colocar la dependencia de la libreria asi:
lib_deps =
  # Using a library name
  EspSoftwareSerial
- En el main.cpp colocar #include <IoTdeviceLib> a mano y seleccionar de la lista deplegable para que actualice automáticamente los directorios de la librería


Contacto:
 manzamb@unicauca.edu.co  