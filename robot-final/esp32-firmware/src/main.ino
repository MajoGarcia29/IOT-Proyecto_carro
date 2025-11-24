#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <NewPing.h>
#include <Wire.h>
#include <MPU6050_light.h>

// ============================
// CONFIGURACIÓN PREPROCESADOR
// ============================
#define WIFI_SSID       "iPhone"
#define WIFI_PASS       "123456789"

#define MQTT_BROKER     "172.20.10.2"  // IP del PC
#define MQTT_PORT       8883
#define MQTT_USER       ""                // Vacío si allow_anonymous
#define MQTT_PASS       ""                // Vacío
#define MQTT_TOPIC_PUB  "robot/sensors"
#define MQTT_TOPIC_SUB  "robot/comando"

// Copia todo el contenido de tu ca.crt aquí:
const char CA_CERT[] PROGMEM = R"EOF(-----BEGIN CERTIFICATE-----
MIIC/zCCAeegAwIBAgIUSjFJJDDJO78N9byEIYtNNxTl+bMwDQYJKoZIhvcNAQEL
BQAwDzENMAsGA1UEAwwETWlDQTAeFw0yNTExMjMxNTQ3NDRaFw0zNTExMjExNTQ3
NDRaMA8xDTALBgNVBAMMBE1pQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK
AoIBAQCxgDWwSwsqzDFAU55fgjimsgMZMl62KZERaakqQNV8/HBf9CqRXFsBLj1F
5zhpUuVo0e0/lopJwZ2bz09v4UgC3ZaD38LEagbX7OZz9XPThvjnjfKtJ8LK+0DB
3gUPHcfQdiKifjEmYI+yQkQdH9rBNlYQVcgaabaBb4B/JrS0jAbdFj/QhGfs8Pi5
+n+3+CkoYPGKuOSb1voCMhsLITr4YTfw/qGBJkKZUvTc6vSpgLyjxUCsyGuUg1Wa
bRy4QcArCVQBRwcKQNC3XnhFeZPSBluikrBvK4Jlc03gows29r5lHDTWeK0Qucv8
Jr1rzn1jdwvHCNWFlkX6NS8sJNB7AgMBAAGjUzBRMB0GA1UdDgQWBBTaFUiOvbkx
6RCPubUc8rqUpTqpijAfBgNVHSMEGDAWgBTaFUiOvbkx6RCPubUc8rqUpTqpijAP
BgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQCpBB1oHKmf/7cAEWx0
bZs+QOXKD5TmLi1N6qpp6WnZtNxYmyfG3AZp2+tWdPVHQ8eZd6yM/E1j/z1QwL84
HSEMZIixAMSspzeZ/esAkcIlMFsHBYtzRaOhcJIyJD3AxBtTB/33JR69wqRmX5f/
FtuoKvLdo9H9F+5L3hsca6Ssc0oCiphTMy0df9io7aGdN1FQfntGtBicdXeehn8F
q2TMnqAbuyqOAVlgr6wcTtm2kltbpjjhNgLrBOOy1CnMIptz13gmk3tbDYFwo534
NjL36UcbZp+BLSP7rU/slRplZGXSWZXgJNrul/g99VaNmDl+i7TdZ4QaBtswxx/F
0z2k
-----END CERTIFICATE-----
)EOF";

// ============================
// PINES MOTORES L298N
// ============================
#define IN1 25
#define IN2 26
#define IN3 27
#define IN4 14
#define ENA 32  // PWM
#define ENB 33  // PWM

int velocidad = 220; // 0-255


// ============================
// SENSOR ULTRASONIDO
// ============================
#define TRIGGER_PIN 12
#define ECHO_PIN    13
#define MAX_DISTANCE 200
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// ============================
// LEDs DIRECCIONALES
// ============================
#define LED_IZQ       19
#define LED_DER       18


// ============================
// OBJETOS MQTT
// ============================
WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);

MPU6050 mpu(Wire);
// ============================
// FUNCIONES GENERALES
// ============================
void ledsOff() {
  digitalWrite(LED_IZQ, LOW);
  digitalWrite(LED_DER, LOW);

}

void adelante() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  ledcWrite(ENA, velocidad);
  ledcWrite(ENB, velocidad);

  digitalWrite(LED_IZQ, HIGH); 
  digitalWrite(LED_DER, HIGH);

}

void atras() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  ledcWrite(ENA, velocidad);
  ledcWrite(ENB, velocidad);
  digitalWrite(LED_IZQ, LOW); 
  digitalWrite(LED_DER, LOW);
  
}

void izquierda() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  ledcWrite(ENA, velocidad / 2);
  ledcWrite(ENB, velocidad);
  digitalWrite(LED_IZQ, HIGH);
   digitalWrite(LED_DER, LOW);
  ;
}

void derecha() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  ledcWrite(ENA, velocidad);
  ledcWrite(ENB, velocidad / 2);
  digitalWrite(LED_IZQ, LOW);
  digitalWrite(LED_DER, HIGH);

}

void detener() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  ledcWrite(ENA, 0);
  ledcWrite(ENB, 0);

  ledsOff();
}

// ============================
// MQTT
// ============================
void connectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Conectando MQTT...");
    if (mqttClient.connect("RobotESP32", MQTT_USER, MQTT_PASS)) {
      Serial.println("conectado!");
      mqttClient.subscribe(MQTT_TOPIC_SUB);
    } else {
      Serial.print("falló, rc="); Serial.println(mqttClient.state());
      delay(5000);
    }
  }
}

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

// ============================
// SETUP
// ============================
void setup() {
  Serial.begin(115200);

  // Pines salida
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(LED_IZQ, OUTPUT); pinMode(LED_DER, OUTPUT);
 

  // PWM LEDC (NUEVA API CORE 3.0)
  ledcAttach(ENA, 5000, 8);
  ledcAttach(ENB, 5000, 8);

  detener(); ledsOff();

  // WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Conectando WiFi");
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");

  // TLS
  wifiClient.setCACert(CA_CERT);

  // MQTT
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  connectMQTT();

  Serial.println("Robot listo!");

  Wire.begin();
  byte status = mpu.begin();
  if (status != 0) Serial.println("Error inicializando MPU6050");
  else Serial.println("MPU6050 listo!");
  mpu.calcGyroOffsets(); // calibración inicial
}

// ============================
// LOOP
// ============================
void loop() {
  mpu.update();
  float ax = mpu.getAccX();
  float ay = mpu.getAccY();
  float az = mpu.getAccZ();
  float roll = mpu.getAngleX();
  float pitch = mpu.getAngleY();

  Serial.print("MPU -> ");
  Serial.print("AX="); Serial.print(ax); Serial.print(" | ");
  Serial.print("AY="); Serial.print(ay); Serial.print(" | ");
  Serial.print("AZ="); Serial.print(az); Serial.print(" | ");
  Serial.print("Roll="); Serial.print(roll); Serial.print(" | ");
  Serial.print("Pitch="); Serial.println(pitch);

  if (!mqttClient.connected()) connectMQTT();
  mqttClient.loop();

  // Medir distancia cada 500ms
  static int dist = 0; 
  static unsigned long lastMeasure = 0;
  if (millis() - lastMeasure > 500) {
    lastMeasure = millis();
    dist = sonar.ping_cm();
    Serial.print("Distancia: "); Serial.println(dist);


    // Evitar obstáculos
    if(dist>0 && dist<20){
      Serial.println("¡Obstáculo! Deteniendo...");
      detener();
      ledsOff();
      delay(500);
      izquierda();
      delay(1000);
      detener();
    }
  
  
    // Publicar datos en MQTT
  String payload = "{\"dist\":" + String(dist) +
                   ",\"ax\":" + String(ax) +
                   ",\"ay\":" + String(ay) +
                   ",\"az\":" + String(az) +
                   ",\"roll\":" + String(roll) +
                   ",\"pitch\":" + String(pitch) + "}";
  mqttClient.publish(MQTT_TOPIC_PUB, payload.c_str());
  }
  delay(50);
}
