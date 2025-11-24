# Proyecto API + MQTT para Control de Carro con ESP32

Este proyecto permite controlar un carro robótico usando una API en Node.js que envía comandos por MQTT hacia una ESP32 conectada a un broker Mosquitto.

## Características

- API en Node.js usando Express
- Cliente MQTT
- Frontend simple en HTML/JS
- Control de motores
- Sensor giroscópico para estabilización y orientación
- Sensor de proximidad para detección de obstáculos
- Instrucciones completas para instalación, ejecución y pruebas

---

## 1. Requisitos

### Software necesario

- **Node.js** (v16 o superior) - [https://nodejs.org](https://nodejs.org)
- **Mosquitto MQTT Broker** - [https://mosquitto.org/download](https://mosquitto.org/download)
- Navegador web (Chrome recomendado)
- **ESP32** (para pruebas reales)
- Arduino IDE o PlatformIO

---

## 2. Estructura del proyecto
```
api-mqtt/
│
├── index.js           # Lógica de la API (Express) y cliente MQTT
├── package.json       # Dependencias de Node.js
├── public/
│   └── index.html     # Interfaz de control (HTML/JS)
├── esp32_mqtt_client.ino  # Código para ESP32
└── README.md
```

---

## 3. Instalación

### 3.1 Clonar el repositorio
```bash
git clone <URL-del-repositorio>
cd api-mqtt
```

### 3.2 Instalar dependencias
```bash
npm install
```

---

## 4. Ejecutar el servidor

### 4.1 Verificar que Mosquitto esté ejecutándose

**Windows:**
```bash
net start mosquitto
```

**Linux/macOS:**
```bash
mosquitto -v
```

Si está funcionando debería mostrar algo como:
```
Opening ipv4 listen socket on port 1883
```

### 4.2 Iniciar la API
```bash
node index.js
```

**Salida esperada:**
```
Conectado al broker MQTT
API corriendo en http://localhost:3000
```

---

## 5. Frontend

Abrir el archivo:
```
public/index.html
```

Se puede abrir con doble clic o desde un servidor local. El frontend usa JavaScript (Fetch API) para enviar peticiones POST a la API de Node.js. Los botones enviarán peticiones a la API.

---

## 6. Endpoints disponibles (API)

### 6.1 Mover el carro

- **Método:** `POST`
- **URL:** `http://localhost:3000/mover`
- **Body (JSON):**
```json
{
  "cmd": "forward"
}
```
- **Publica en el tópico MQTT:** `carro/comandos`
- **Valores válidos:** `"forward"`, `"left"`, `"right"`, `"backward"`, `"stop"`

### 6.2 Cambiar velocidad

- **Método:** `POST`
- **URL:** `http://localhost:3000/velocidad`
- **Body (JSON):**
```json
{
  "valor": 150
}
```
- **Publica en:** `carro/velocidad`
- **Valor numérico en el rango:** `0–255`

---

## 7. Tópicos MQTT usados

| Acción       | Tópico           | Payload                                          |
|--------------|------------------|--------------------------------------------------|
| Movimiento   | `carro/comandos` | `"forward"`, `"left"`, `"right"`, `"backward"`, `"stop"` |
| Velocidad    | `carro/velocidad`| Valor numérico (0–255)                           |
| Giroscopio   | `carro/gyro`     | Datos de orientación (JSON)                      |
| Proximidad   | `carro/distancia`| Distancia en cm (numérico)                       |

---

## 8. Sensores implementados

### 8.1 Sensor Giroscópico

El sensor giroscópico (MPU6050 o similar) permite:
- Detectar la orientación del carro
- Estabilización durante el movimiento
- Detección de inclinación

**Tópico MQTT:** `carro/gyro`

**Formato de datos:**
```json
{
  "x": 0.5,
  "y": -0.3,
  "z": 9.8
}
```

### 8.2 Sensor de Proximidad

El sensor de proximidad (HC-SR04 o similar) permite:
- Detección de obstáculos
- Medición de distancia
- Frenado automático ante obstáculos

**Tópico MQTT:** `carro/distancia`

**Formato de datos:**
```json
{
  "distancia": 25
}
```

---

## 9. Código para ESP32

La ESP32 debe:
- Conectarse al WiFi
- Conectarse al broker MQTT (usando la IP del computador, no `localhost`)
- Suscribirse a `carro/comandos`, `carro/velocidad`
- Publicar datos de sensores en `carro/gyro` y `carro/distancia`
- Controlar motores según los mensajes recibidos

**Nota:** Para compilar el código en Arduino IDE, necesitarás instalar las siguientes librerías:
- `PubSubClient` (MQTT)
- `MPU6050` o `Adafruit_MPU6050` (Giroscopio)
- `NewPing` o librería nativa para HC-SR04 (Proximidad)

El código de ejemplo se encuentra en el archivo `esp32_mqtt_client.ino`.

---

## 10. Cómo funciona el sistema de comandos

La interfaz web envía comandos como letras (F, B, L, R, S) y en el código de la ESP32 hay una función que interpreta cada letra y llama a la acción correspondiente.

### 10.1 La interfaz web envía la letra

En `control.html` tienes:
```javascript
document.getElementById("up").onclick = () => send("F");
document.getElementById("down").onclick = () => send("B");
document.getElementById("left").onclick = () => send("L");
document.getElementById("right").onclick = () => send("R");
document.getElementById("stop").onclick = () => send("S");
```

- `send("F")` envía la letra "F" al servidor Node.js vía Socket.IO
- "F" significa adelante, "B" significa atrás, "L" izquierda, "R" derecha y "S" detener

### 10.2 Node.js recibe la letra y la publica por MQTT

En `server.js`:
```javascript
io.on('connection', (socket) => {
    console.log("UI conectada");
    socket.on("cmd", (cmd) => {
        console.log("Comando recibido:", cmd);
        client.publish(TOPIC_CMD, cmd);
    });
});
```

- `cmd` es la letra recibida de la interfaz (por ejemplo "F")
- Se publica en el topic MQTT `robot/comando`
- Ahí es donde aparece en la consola del servidor:
```
  Comando recibido: F
```

### 10.3 ESP32 recibe la letra desde MQTT

En tu ESP32:
```cpp
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String comando = "";
  for (int i=0; i<length; i++) comando += (char)payload[i];
  Serial.print("Comando recibido: "); Serial.println(comando);
  if (comando == "F") adelante();
  else if (comando == "B") atras();
  else if (comando == "L") izquierda();
  else if (comando == "R") derecha();
  else if (comando == "S") detener();
}
```

- `payload` contiene la letra que publicó Node.js en MQTT
- Se convierte a `String comando`
- Luego, hay una serie de `if` que mapea cada letra a una función de movimiento:

| Letra | Función llamada |
|-------|-----------------|
| F     | adelante()      |
| B     | atras()         |
| L     | izquierda()     |
| R     | derecha()       |
| S     | detener()       |

- `Serial.println("Comando recibido: " + comando);` muestra en el monitor serie la letra que llegó

---

## 11. Pruebas sin ESP32

Se puede validar la funcionalidad de la mensajería MQTT con las herramientas de Mosquitto.

**Publicar mensaje:**
```bash
mosquitto_pub -t carro/comandos -m "forward"
```

**Escuchar mensajes:**
```bash
mosquitto_sub -t carro/comandos
```

**Simular datos de sensores:**
```bash
mosquitto_pub -t carro/distancia -m '{"distancia": 15}'
mosquitto_pub -t carro/gyro -m '{"x": 0.1, "y": 0.2, "z": 9.8}'
```

---

## 12. Notas importantes

- La ESP32 no debe usar `"localhost"`; necesita la IP del PC en la misma red
- Para obtener la IP (Host):

| Sistema Operativo | Comando         | Campo a buscar  |
|-------------------|-----------------|-----------------|
| Windows           | `ipconfig`      | "IPv4 Address"  |
| Linux/macOS       | `ip a` o `ifconfig` | "inet"      |

- Si el frontend funciona y la API recibe las peticiones, pero no aparece nada en la ESP32, significa que la ESP32 aún no se ha conectado al broker (revisar configuración de IP y credenciales WiFi)
- Los sensores deben ser inicializados correctamente en el código de la ESP32
- Se recomienda calibrar el giroscopio al inicio

---

## 13. Pasos que debe seguir cualquier colaborador

1. Instalar Node.js y Mosquitto
2. Clonar el repositorio
3. Ejecutar `npm install`
4. Ejecutar `node index.js`
5. Abrir `public/index.html`
6. Conectar la ESP32 con el código MQTT y la IP correcta
7. Conectar sensores giroscópico y de proximidad según el esquema de pines
8. Verificar lecturas de sensores en los tópicos MQTT correspondientes

---

## 14. Esquema de conexión de sensores

### Sensor Giroscópico (MPU6050)

| Pin ESP32 | Pin MPU6050 |
|-----------|-------------|
| 3.3V      | VCC         |
| GND       | GND         |
| GPIO 21   | SDA         |
| GPIO 22   | SCL         |

### Sensor de Proximidad (HC-SR04)

| Pin ESP32 | Pin HC-SR04 |
|-----------|-------------|
| 5V        | VCC         |
| GND       | GND         |
| GPIO 18   | TRIG        |
| GPIO 19   | ECHO        |

---

## 15. Troubleshooting

### Problema: ESP32 no se conecta al broker
- Verificar que la IP del broker sea correcta
- Comprobar que ambos dispositivos estén en la misma red
- Revisar credenciales WiFi

### Problema: Sensores no envían datos
- Verificar conexiones físicas
- Comprobar que las librerías estén instaladas
- Revisar inicialización en el código

### Problema: API no responde
- Verificar que Node.js esté ejecutándose
- Comprobar que el puerto 3000 no esté ocupado
- Revisar logs del servidor

---

## Licencia

Este proyecto está bajo licencia MIT.

## Contribuciones

Las contribuciones son bienvenidas. Por favor, abre un issue o pull request para sugerencias o mejoras.
