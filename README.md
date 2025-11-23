# Proyecto API + MQTT para Control de Carro con ESP32
 
Este proyecto permite controlar un carro robótico usando una API en Node.js que envía comandos por MQTT hacia una ESP32 conectada a un broker Mosquitto.Incluye:API en Node.js usando ExpressCliente MQTTFrontend simple en HTML/JSInstrucciones completas para instalación, ejecución y pruebas

## 1. Requisitos: 
Software necesarioNode.js (v16 o superior) https://nodejs.orgMosquitto MQTT Broker https://mosquitto.org/downloadNavegador web (Chrome recomendado)ESP32 (solo para pruebas reales)Arduino IDE o PlatformIO

## 2. Estructura del proyectoapi-mqtt/

│── index.js           // Lógica de la API (Express) y cliente MQTT

│── package.json       // Dependencias de Node.js

│── public/
│     └── index.html   // Interfaz de control (HTML/JS)

└── README.md

## 3.  Instalación3.1 Clonar el repositoriogit clone <URL-del-repositorio>
### 3.1 cd api-mqtt
### 3.2 Instalar dependenciasnpm install

## 4.  Ejecutar el servidor4.1 Verificar que Mosquitto esté ejecutándoseWindows:net start mosquitto
Linux/macOS:mosquitto -v
Si está funcionando debería mostrar algo como:Opening ipv4 listen socket on port 1883
### 4.2 Iniciar la APInode index.js
Salida esperada:Conectado al broker MQTT
API corriendo en http://localhost:3000

## 5.  FrontendAbrir el archivo:public/index.html
Se puede abrir con doble clic o desde un servidor local. El frontend usa JavaScript (Fetch API) para enviar peticiones POST a la API de Node.js.Los botones enviarán peticiones a la API.

## 6.  Endpoints disponibles (API)
### 6.1 Mover el carroMétodo: 

POSTURL: http://localhost:3000/moverBody (JSON):{
  "cmd": "forward"
}
Publica en el tópico MQTT: carro/comandos (Valores válidos: "forward", "left", "right", "backward", "stop")6.2 Cambiar velocidadMétodo: POSTURL: http://localhost:3000/velocidadBody (JSON):{
  "valor": 150
}

Publica en: carro/velocidad (Valor numérico en el rango 0–255)7. Tópicos MQTT usadosAcciónTópicoPayloadMovimientocarro/comandos"forward", "left", "right", "backward", "stop"Velocidadcarro/velocidadvalor numérico (0–255)8.  Código para ESP32La ESP32 debe:Conectarse al WiFiConectarse al broker MQTT (usando la IP del computador, no localhost)Suscribirse a carro/comandos y carro/velocidadControlar motores según los mensajes recibidosNota: Para compilar el código en Arduino IDE, necesitarás instalar la librería PubSubClient. El código de ejemplo se encuentra en el archivo esp32_mqtt_client.ino.

## 9.  Pruebas sin ESP32
Se puede validar la funcionalidad de la mensajería MQTT con las herramientas de Mosquitto.Publicar mensaje:mosquitto_pub -t carro/comandos -m "forward"
Escuchar mensajes:mosquitto_sub -t carro/comandos

## 10.  Notas importantes
La ESP32 no debe usar "localhost"; necesita la IP del PC en la misma red.Para obtener la IP (Host):Sistema OperativoComandoCampo a buscarWindowsipconfig"IPv4 Address"Linux/macOSip a (o ifconfig)"inet"Si el frontend funciona y la API recibe las peticiones, pero no aparece nada en la ESP32, significa que la ESP32 aún no se ha conectado al broker (revisar configuración de IP y credenciales WiFi).

## 11. Pasos que debe seguir cualquier colaborador
* Instalar Node.js y MosquittoClonar el repositorioEjecutar npm installEjecutar node index.js
* Abrir public/index.htmlConectar la ESP32 con el código MQTT y la IP correcta.
