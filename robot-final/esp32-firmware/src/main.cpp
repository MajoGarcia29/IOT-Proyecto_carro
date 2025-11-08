
// main.cpp
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "config.h"
#include <Wire.h>
#include "MPU6050.h"

// HC-SR04 pins
const int TRIG_PIN = 18;
const int ECHO_PIN = 19;

// MPU6050
MPU6050 mpu;

// WiFi and MQTT
WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);

unsigned long lastPublish = 0;
const unsigned long PUBLISH_INTERVAL = 1000; // ms

// helper: measure distance (cm)
long measureDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000UL); // timeout 30 ms
  if (duration == 0) return -1;
  long cm = duration / 58;
  return cm;
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // handle move commands (simple JSON expected)
  String t = String(topic);
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];

  if (t == MQTT_TOPIC_COMMANDS) {
    // ejemplo: {"action":"forward","speed":120}
    // Aquí deberías traducir a PWM / H-bridge
    Serial.println("Comando recibido:");
    Serial.println(msg);
    // TODO: agregar control real de motores según tu driver
  }
}

void connectWifi() {
  Serial.print("Conectando a WiFi ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 30) {
    delay(500);
    Serial.print(".");
    tries++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi conectado");
    Serial.print("IP: "); Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nNo se pudo conectar a WiFi");
  }
}

void connectMQTT() {
  if (mqttClient.connected()) return;
  Serial.print("Conectando MQTT...");
  // Config del cliente TLS: fingerprint o CA
#if USE_CERTIFICATE
  wifiClient.setCACert(MQTT_CA_CERT);
#elif USE_FINGERPRINT
  // setFingerprint deprecated in some libs, but we include for placeholder
  wifiClient.setFingerprint(MQTT_FINGERPRINT);
#endif

  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);

  int attempts = 0;
  while (!mqttClient.connected() && attempts < 10) {
    String clientId = "esp32-" + String((uint32_t)esp_random(), HEX);
    Serial.print("Intentando conectar MQTT, clientId=");
    Serial.println(clientId);
    if (mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
      Serial.println("Conectado a MQTT");
      mqttClient.subscribe(MQTT_TOPIC_COMMANDS);
    } else {
      Serial.print("Fallo, rc=");
      Serial.println(mqttClient.state());
      delay(2000);
      attempts++;
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Wire.begin(); // SDA, SCL default
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 no responde!");
  } else {
    Serial.println("MPU6050 listo");
  }

  connectWifi();
  connectMQTT();
  lastPublish = millis();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWifi();
  if (!mqttClient.connected()) connectMQTT();
  mqttClient.loop();

  unsigned long now = millis();
  if (now - lastPublish >= PUBLISH_INTERVAL) {
    lastPublish = now;

    long dist = measureDistanceCm();
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getAcceleration(&ax, &ay, &az);
    mpu.getRotation(&gx, &gy, &gz);

    // Build JSON payloads
    char payload1[128];
    if (dist >= 0) {
      snprintf(payload1, sizeof(payload1),
        "{\"timestamp\":%lu,\"distance_cm\":%ld}", now, dist);
      mqttClient.publish(MQTT_TOPIC_PROXIMITY, payload1, true);
      Serial.print("Publicado proximidad: "); Serial.println(payload1);
    } else {
      // Optional: publish invalid reading
      snprintf(payload1, sizeof(payload1),
        "{\"timestamp\":%lu,\"distance_cm\":null}", now);
      mqttClient.publish(MQTT_TOPIC_PROXIMITY, payload1, true);
      Serial.println("Proximidad: timeout");
    }

    char payload2[256];
    snprintf(payload2, sizeof(payload2),
      "{\"timestamp\":%lu,\"accel\":{\"x\":%d,\"y\":%d,\"z\":%d},\"gyro\":{\"x\":%d,\"y\":%d,\"z\":%d}}",
      now, ax, ay, az, gx, gy, gz);
    mqttClient.publish(MQTT_TOPIC_IMU, payload2, true);
    Serial.print("Publicado IMU: "); Serial.println(payload2);
  }
}
