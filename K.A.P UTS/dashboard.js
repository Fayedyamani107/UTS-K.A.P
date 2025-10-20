// File: dashboard.js

// CONFIG — Broker harus sama dengan yang di CPP dan menggunakan port WebSocket (8080)
const BROKER_HOST = "test.mosquitto.org"; 
const BROKER_WS_PORT = 8080;
const CLIENT_ID = "web-dashboard-" + Math.floor(Math.random()*1000);

// TOPICS
const TEMP_HUM_TOPIC = "/smart/home/data/suhu_lembap";
const DISTANCE_TOPIC = "/smart/home/data/garasi/jarak";
const LIGHT_CONTROL_TOPIC = "/smart/home/kontrol/lampu";
const DOOR_CONTROL_TOPIC = "/smart/home/kontrol/pintu";

// UI elements
const brokerStatusEl = document.getElementById("brokerStatus");
const connectBtn = document.getElementById("connectBtn");
const disconnectBtn = document.getElementById("disconnectBtn");
const tempEl = document.getElementById("tempVal");
const humEl = document.getElementById("humVal");
const distEl = document.getElementById("distVal");
const logEl = document.getElementById("log");
const btnOn = document.getElementById("btnOn");
const btnOff = document.getElementById("btnOff");
const lampStatusEl = document.getElementById("lampStatus");
const servoRange = document.getElementById("servoRange");
const servoVal = document.getElementById("servoVal");
const servoSend = document.getElementById("servoSend");

let client = null;

function addLog(msg) {
    const t = new Date().toLocaleTimeString();
    // Memasukkan log terbaru di bagian atas
    logEl.innerHTML = `[${t}] ${msg}<br>` + logEl.innerHTML; 
}

function connect() {
    if (client && client.connected) return;

    // Protokol ws:// digunakan untuk koneksi WebSocket (browser)
    const url = `ws://${BROKER_HOST}:${BROKER_WS_PORT}/`; 
    addLog("Connecting to " + url);
    brokerStatusEl.textContent = "Connecting...";
    
    // Inisialisasi koneksi MQTT
    client = mqtt.connect(url, {
        clientId: CLIENT_ID,
        keepalive: 30,
        reconnectPeriod: 0, // Jangan auto-reconnect
    });

    client.on("connect", function() {
        brokerStatusEl.textContent = "Connected";
        brokerStatusEl.className = "status connected";
        connectBtn.disabled = true;
        disconnectBtn.disabled = false;
        addLog("Connected to broker");

        // Subscribe ke semua topik data sensor dan kontrol
        client.subscribe(TEMP_HUM_TOPIC);
        client.subscribe(DISTANCE_TOPIC);
        client.subscribe(LIGHT_CONTROL_TOPIC); 
    });

    client.on("close", function() {
        brokerStatusEl.textContent = "Disconnected";
        brokerStatusEl.className = "status disconnected";
        connectBtn.disabled = false;
        disconnectBtn.disabled = true;
        addLog("Connection closed");
    });
    
    client.on("error", function(err) {
        addLog(`Connection Error: ${err.message}`);
        // Matikan client saat error
        client.end(); 
    });

    client.on("message", function(topic, payload) {
        const p = payload.toString();
        // addLog(`Msg ${topic} -> ${p}`); // Terlalu banyak log jika setiap 5 detik

        if (topic === TEMP_HUM_TOPIC) {
            const parts = p.split(",");
            if (parts.length >= 2) {
                tempEl.textContent = parts[0];
                humEl.textContent = parts[1];
            }
        } else if (topic === DISTANCE_TOPIC) {
            distEl.textContent = p;
        } else if (topic === LIGHT_CONTROL_TOPIC) {
            // Ini akan menampilkan ON/OFF
            lampStatusEl.textContent = p; 
        }
    });
}

function disconnect() {
    if (client) {
        client.end();
        addLog("Disconnected manually");
    }
}

// ----------------- EVENT LISTENERS -----------------

// Tombol Connect/Disconnect
connectBtn.addEventListener("click", connect);
disconnectBtn.addEventListener("click", disconnect);

// Kontrol Lampu (Relay)
btnOn.addEventListener("click", () => {
    if (client && client.connected) {
        client.publish(LIGHT_CONTROL_TOPIC, "ON");
        addLog("Lamp ON sent");
        lampStatusEl.textContent = "ON"; 
    }
});

btnOff.addEventListener("click", () => {
    if (client && client.connected) {
        client.publish(LIGHT_CONTROL_TOPIC, "OFF");
        addLog("Lamp OFF sent");
        lampStatusEl.textContent = "OFF";
    }
});

// Kontrol Pintu (Servo)
servoRange.addEventListener("input", () => {
    servoVal.textContent = servoRange.value;
});

servoSend.addEventListener("click", () => {
    const angle = servoRange.value;
    if (client && client.connected) {
        // Kirim nilai sudut sebagai String
        client.publish(DOOR_CONTROL_TOPIC, String(angle)); 
        addLog("Servo angle sent: " + angle);
    }
});

// Koneksi otomatis saat dashboard dimuat
window.onload = connect;