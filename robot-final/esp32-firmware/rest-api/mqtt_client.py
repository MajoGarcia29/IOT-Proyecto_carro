# mqtt_client.py
import paho.mqtt.client as mqtt
import json
import threading
import time

MQTT_HOST = "mqtt.tu-broker.com"
MQTT_PORT = 8883
MQTT_USER = "usuario"
MQTT_PASS = "clave"
TOPIC_PROX = "robot/sensors/proximity"
TOPIC_IMU = "robot/sensors/imu"
TOPIC_COMMAND = "robot/commands/move"

class MQTTClient:
    def __init__(self):
        self.client = mqtt.Client()
        self.client.username_pw_set(MQTT_USER, MQTT_PASS)
        # TLS
        self.client.tls_set()  # usa CA del sistema; ajustar si es necesario
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.latest_proximity = None
        self.latest_imu = None
        self.last_command = None
        self._connected = False

    def connect(self):
        def _run():
            self.client.connect(MQTT_HOST, MQTT_PORT, 60)
            self.client.loop_forever()
        t = threading.Thread(target=_run)
        t.daemon = True
        t.start()
        # esperar breve tiempo
        time.sleep(1)

    def on_connect(self, client, userdata, flags, rc):
        print("MQTT conectado rc=", rc)
        self._connected = True
        client.subscribe(TOPIC_PROX)
        client.subscribe(TOPIC_IMU)

    def on_message(self, client, userdata, msg):
        try:
            payload = msg.payload.decode('utf-8')
            if msg.topic == TOPIC_PROX:
                self.latest_proximity = json.loads(payload)
            elif msg.topic == TOPIC_IMU:
                self.latest_imu = json.loads(payload)
        except Exception as e:
            print("Error parseando mensaje:", e)

    def publish_command(self, command_obj):
        payload = json.dumps(command_obj)
        self.client.publish(TOPIC_COMMAND, payload)
        print("Publicado comando:", payload)

    def is_connected(self):
        return self._connected

