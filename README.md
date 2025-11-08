# Robot 2WD - Proyecto Final

## Contenido
- Firmware ESP32 (sensores + MQTT)
- REST API (Flask) que expone endpoints y publica comandos MQTT
- Documentación, diagramas, colección Postman, presentación

## Requisitos
- ESP32 board
- HC-SR04 (o sensor de proximidad equivalente)
- MPU6050 (I2C)
- Driver de motores (L298N, TB6612, etc)
- Broker MQTT con TLS (ej. Mosquitto con certificados)
- Python 3.9+, pip

## Pasos rápidos
1. Ajustar `esp32-firmware/src/config.h` con WIFI y datos MQTT y subir el sketch al ESP32.
2. Instalar dependencias en `rest-api/`: pip install -r rest-api/requirements.txt
3. Ejecutar API: python rest-api/app.py
4. Importar `rest-api/postman_collection.json` en Postman. Probar endpoints.
5. Compilar en IDE para ver uso de memoria y pegar la salida en `docs/README.md`.

## Entrega
- Presentación en `presentation/` (slides.md + speaker_notes.md)
