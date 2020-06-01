Librería IoTcomLib
Esta libreria encapsula algunos métodos para comunicación de datos entre dispositivos, gateway y servidores IoT, así como métodos para la interacción entre los mismos.

Para usarla: 
- Se copian los archivos en la carpeta /lib/
- instalar la librería WIFIManager con Platformio en la etiqueta de librerías
- En el platformio.ini del proyecto colocar la dependencia de la libreria asi:
lib_deps =
  # Using a library name
  WifiManager
- En el main.cpp colocar #include <IoTcomLib> a mano y seleccionar de la lista deplegable para que actualice automáticamente los directorios de la librería

Contacto:
 manzamb@unicauca.edu.co  