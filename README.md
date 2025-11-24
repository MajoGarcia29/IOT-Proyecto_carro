# Proyecto API + MQTT para Control de Carro con ESP32

Este proyecto permite controlar un carro robótico usando una API en Node.js que envía comandos por MQTT hacia una ESP32 conectada a un broker Mosquitto.

## Características

- API en Node.js 
- Cliente MQTT
- Frontend simple en HTML/JS
- Control de motores
- Sensor giroscópico para estabilización y orientación
- Sensor de proximidad para detección de obstáculos
- Instrucciones completas para instalación, ejecución y pruebas

---

## 1. Requisitos

### 1.1 Software necesario

- **Node.js** (v16 o superior) - [https://nodejs.org](https://nodejs.org)
- **Mosquitto MQTT Broker** - [https://mosquitto.org/download](https://mosquitto.org/download)
- **OpenSSL** (para generar certificados TLS)
- Navegador web (Chrome recomendado)
- **ESP32** (para pruebas reales)
- Arduino IDE o PlatformIO

### 1.2 Crear carpeta de trabajo para certificados

En tu PC crea una carpeta `robot-project/certs`:

**Linux/macOS:**
```bash
mkdir -p ~/robot-project/certs
cd ~/robot-project/certs
```

**Windows (PowerShell):**
```powershell
mkdir C:\robot-project\certs
cd C:\robot-project\certs
```

### 1.3 Generar CA y certificado servidor (OpenSSL)

Copia y pega estos comandos (ajusta `-subj` si quieres nombres distintos). El Common Name (CN) del servidor puede ser la IP local si usas IP directa (ej. 192.168.1.100) o localhost.
```bash
# 1) Crear clave de la CA
openssl genrsa -out ca.key 2048

# 2) Crear certificado auto-firmado de la CA
openssl req -x509 -new -nodes -key ca.key -sha256 -days 3650 -out ca.crt -subj "/CN=MiCA"

# 3) Crear clave del servidor (Mosquitto)
openssl genrsa -out server.key 2048

# 4) Crear CSR (Certificate Signing Request) para el servidor
# Reemplaza CN en -subj por la IP de tu PC en la red local si vas a conectar por IP (ej: 192.168.1.100)
openssl req -new -key server.key -out server.csr -subj "/CN=192.168.1.100"

# 5) Firmar el CSR con la CA (generar server.crt)
openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt -days 365 -sha256
```

Al final tendrás en `certs/`:
- `ca.crt` (certificado de la CA)
- `server.crt` (cert del servidor)
- `server.key` (clave privada del servidor)

**Nota:** Para pruebas locales es suficiente; si quieres que el ESP32 valide el broker, usa `ca.crt` en el ESP32.

### 1.4 Configurar Mosquitto con TLS

Detener y reiniciar el servicio de Mosquitto:

**Windows (PowerShell como Administrador):**
```powershell
# Detener el servicio
NET stop mosquitto

# Iniciar el servicio
net start mosquitto

# Verificar que el puerto 8883 esté escuchando
netstat -an | findstr 8883

# Si no aparece, ejecutar Mosquitto manualmente con configuración
cd "C:\Program Files\mosquitto"
mosquitto -c C:/robot-project/mosquitto.conf -v
```

**Linux/macOS:**
```bash
# Detener el servicio
sudo systemctl stop mosquitto

# Iniciar el servicio
sudo systemctl start mosquitto

# Verificar estado
sudo systemctl status mosquitto

# Verificar puerto
netstat -an | grep 8883
```

---

## 2. Instalación del proyecto

### 2.1 Clonar el repositorio
```bash
git clone <URL-del-repositorio>
cd api-mqtt
```

### 2.2 Instalar dependencias
```bash
npm install
```

---

## 3. Ejecutar el servidor

### 3.1 Verificar que Mosquitto esté ejecutándose

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
Opening ipv4 listen socket on port 8883
```

### 3.2 Iniciar la API
```bash
node index.js
```

**Salida esperada:**
```
Conectado al broker MQTT
API corriendo en http://localhost:3000
```

---

## 4. Frontend

Abrir el archivo:
```
public/index.html
```

Se puede abrir con doble clic o desde un servidor local. El frontend usa JavaScript (Fetch API) para enviar peticiones POST a la API de Node.js. Los botones enviarán peticiones a la API.

---

## 5. Endpoints disponibles 

### 5.1 Mover el carro

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

### 5.2 Cambiar velocidad

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

## 6. Tópicos MQTT usados

| Acción       | Tópico           | Payload                                          |
|--------------|------------------|--------------------------------------------------|
| Movimiento   | `carro/comandos` | `"forward"`, `"left"`, `"right"`, `"backward"`, `"stop"` |
| Velocidad    | `carro/velocidad`| Valor numérico (0–255)                           |
| Giroscopio   | `carro/gyro`     | Datos de orientación (JSON)                      |
| Proximidad   | `carro/distancia`| Distancia en cm (numérico)                       |

---

## 7. Sensores implementados

### 7.1 Sensor Giroscópico

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

### 7.2 Sensor de Proximidad

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

## 8. Código para ESP32

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

## 9. Cómo funciona el sistema de comandos

La interfaz web envía comandos como letras (F, B, L, R, S) y en el código de la ESP32 hay una función que interpreta cada letra y llama a la acción correspondiente.

### 9.1 La interfaz web envía la letra

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

### 9.2 Node.js recibe la letra y la publica por MQTT

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

### 9.3 ESP32 recibe la letra desde MQTT

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

## 10. Pruebas sin ESP32

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

## 11. Pasos que debe seguir cualquier colaborador

1. Instalar Node.js y Mosquitto
2. Generar certificados TLS según sección 1.2 y 1.3
3. Configurar Mosquitto con TLS (sección 1.4)
4. Clonar el repositorio
5. Ejecutar `npm install`
6. Ejecutar `node index.js`
7. Abrir `public/index.html`
8. Conectar la ESP32 con el código MQTT y la IP correcta
9. Conectar sensores giroscópico y de proximidad según el esquema de pines
10. Verificar lecturas de sensores en los tópicos MQTT correspondientes

---

## 12. Esquema de conexión de sensores

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

## 13. Troubleshooting

### Problema: ESP32 no se conecta al broker
- Verificar que la IP del broker sea correcta
- Comprobar que ambos dispositivos estén en la misma red
- Revisar credenciales WiFi
- Verificar que el puerto 8883 (TLS) o 1883 esté accesible

### Problema: Sensores no envían datos
- Verificar conexiones físicas
- Comprobar que las librerías estén instaladas
- Revisar inicialización en el código

### Problema: API no responde
- Verificar que Node.js esté ejecutándose
- Comprobar que el puerto 3000 no esté ocupado
- Revisar logs del servidor

### Problema: Mosquitto no inicia con TLS
- Verificar que los certificados existan en la ruta especificada
- Revisar permisos de lectura de los archivos de certificados
- Comprobar la configuración en `mosquitto.conf`
- Ejecutar `mosquitto -c ruta/mosquitto.conf -v` para ver errores detallados

---


## Miembros del Proyecto
- María José García
- Axel Daniel Bedoya
- Sara Nicole Zuluaga

