// =======================
// server.js COMPLETO
// =======================
const express = require('express');
const bodyParser = require('body-parser');
const mqtt = require('mqtt');
const cors = require('cors');
const fs = require('fs');
const http = require('http');

const app = express();
app.use(bodyParser.json());
app.use(cors());
app.use(express.static('public'));

// Servir archivos estáticos (HTML, JS, CSS) GET  
app.use(express.static('public'));

const server = http.createServer(app);
const { Server } = require("socket.io");
const io = new Server(server);

const PORT = 3000;

// =======================
// CONFIG MQTT
// =======================
const MQTT_HOST = 'mqtts://172.20.10.2:8883';
const MQTT_OPTIONS = {
    username: '',
    password: '',
    ca: fs.readFileSync('certs/ca.crt'),
    rejectUnauthorized: true
};

const client = mqtt.connect(MQTT_HOST, MQTT_OPTIONS);

// =======================
// TOPICS MQTT
// =======================
const TOPIC_CMD = 'robot/comando';
const TOPIC_SENSORS = 'robot/sensors';

// =======================
// VARIABLES SENSOR
// =======================
let lastSensors = null;

// =======================
// MQTT EVENTS
// =======================
client.on('connect', () => {
    console.log('MQTT Conectado!');
    client.subscribe(TOPIC_SENSORS);
});

client.on('message', (topic, message) => {
    if (topic === TOPIC_SENSORS) {
        try {
            lastSensors = JSON.parse(message.toString());
            io.emit("sensors", lastSensors); // Enviar a la UI
        } catch (e) {
            console.error("Error JSON:", e);
        }
    }
});

// =======================
// SOCKET.IO: comandos desde el navegador
// =======================
io.on('connection', (socket) => {
    console.log("UI conectada");

    socket.on("cmd", (cmd) => {
        console.log("Comando recibido:", cmd);
        client.publish(TOPIC_CMD, cmd);
    });
});

// =======================
// ENDPOINTS REST
// =======================

// GET Healthcheck
app.get('/api/v1/healthcheck', (req, res) => {
    const status = {
        server: 'ok',
        mqtt: client.connected ? 'connected' : 'disconnected',
        lastSensorData: lastSensors
    };
    res.json(status);
});

// POST Move
app.post('/api/v1/move', (req, res) => {
    const cmd = req.body.cmd; // Espera { "cmd": "F" } por ejemplo
    if (!cmd) return res.status(400).json({ error: "Falta el campo 'cmd'" });

    client.publish(TOPIC_CMD, cmd);
    res.json({ status: "ok", commandSent: cmd });
});

// GET Move rápido (opcional)
app.get('/api/v1/move/:cmd', (req, res) => {
    const cmd = req.params.cmd;
    client.publish(TOPIC_CMD, cmd);
    res.json({ status: "ok", commandSent: cmd });
});


// =======================
// INICIAR SERVIDOR
// =======================
server.listen(PORT, () => {
    console.log(Servidor web en http://localhost:${PORT});
});
