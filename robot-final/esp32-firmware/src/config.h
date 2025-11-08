
// config.h
#pragma once

// WiFi
#define WIFI_SSID "TU_SSID"
#define WIFI_PASSWORD "TU_PASSWORD"

// MQTT broker
#define MQTT_HOST "mqtt.tu-broker.com"   // cambiar al host real
#define MQTT_PORT 8883
#define MQTT_USER "usuario"
#define MQTT_PASS "clave"

// Topics
#define MQTT_TOPIC_PROXIMITY "robot/sensors/proximity"
#define MQTT_TOPIC_IMU "robot/sensors/imu"
#define MQTT_TOPIC_COMMANDS "robot/commands/move"

// TLS config: elegir uno
#define USE_CERTIFICATE 1
//#define USE_FINGERPRINT 1

#if USE_CERTIFICATE
const char MQTT_CA_CERT[] = R"EOF(
-----BEGIN CERTIFICATE-----
...PEGA-AQUI-EL-CA-DEL-BROKER...
-----END CERTIFICATE-----
)EOF";
#endif

#if USE_FINGERPRINT
const uint8_t MQTT_FINGERPRINT[20] = { /* SHA1 fingerprint bytes */ };
#endif
