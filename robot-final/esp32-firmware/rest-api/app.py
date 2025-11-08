
# app.py
from flask import Flask, jsonify, request
from mqtt_client import MQTTClient
from flask_cors import CORS
import time

app = Flask(__name__)
CORS(app)

mqtt = MQTTClient()
mqtt.connect()

@app.route('/api/v1/healthcheck', methods=['GET'])
def healthcheck():
    return jsonify({
        "status": "ok",
        "mqtt_connected": mqtt.is_connected(),
        "timestamp": int(time.time()*1000)
    })

@app.route('/api/v1/move', methods=['POST'])
def move():
    data = request.get_json(force=True)
    # Validate minimal fields
    if not data or 'action' not in data:
        return jsonify({"error":"payload must include 'action'"}), 400
    mqtt.publish_command(data)
    mqtt.last_command = data
    return jsonify({"status":"published","command":data}), 200

@app.route('/api/v1/move', methods=['GET'])
def get_move():
    return jsonify({"last_command": mqtt.last_command}), 200

@app.route('/api/v1/sensors/proximity', methods=['GET'])
def get_proximity():
    return jsonify({"latest": mqtt.latest_proximity}), 200

@app.route('/api/v1/sensors/imu', methods=['GET'])
def get_imu():
    return jsonify({"latest": mqtt.latest_imu}), 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
