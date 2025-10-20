#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <ESP32Servo.h>

// ================== PIN DEFINITIONS (KOREKSI FINAL UNTUK STABILITAS) ==================
// Pin lama Anda (34, 15, 14) diganti karena konflik boot atau Input Only.
#define DHT_PIN 25      // KOREKSI: Pindah dari GPIO 34 (Input Only) ke GPIO 25 (I/O Aman)
#define DHT_TYPE DHT11
#define TRIG_PIN 19
#define ECHO_PIN 18
#define RELAY_PIN 26    // KOREKSI: Pindah dari GPIO 15 (Pin Boot) ke GPIO 26 (I/O Aman)
#define SERVO_PIN 13    // KOREKSI: Pindah dari GPIO 14 ke GPIO 13 (PWM Stabil)
#define SDA_PIN 21      // Pin I2C Default ESP32
#define SCL_PIN 22      // Pin I2C Default ESP32
    

// ================== WIFI & MQTT CONFIG ==================
const char* ssid = "Dosen";
const char* password = "dosenjaya";
const char* mqtt_broker = "test.mosquitto.org";
const int mqtt_port = 1883;
const char* client_id = "esp32-smart-home";

// ================== MQTT TOPICS ==================
const char* TEMP_HUM_TOPIC = "/smart/home/data/suhu_lembap";
const char* DISTANCE_TOPIC = "/smart/home/data/garasi/jarak";
const char* LIGHT_CONTROL_TOPIC = "/smart/home/kontrol/lampu";
const char* DOOR_CONTROL_TOPIC = "/smart/home/kontrol/pintu";

// ================== OBJECTS ==================
WiFiClient wifiClient;
PubSubClient client(wifiClient);
// Alamat I2C umum. Jika LCD tidak nyala, coba 0x3F atau 0x20.
LiquidCrystal_I2C lcd(0x27, 16, 2); 
DHT dht(DHT_PIN, DHT_TYPE);
Servo doorServo;

// ================== VARIABLES ==================
float temperature_C = 0.0;
float humidity = 0.0;
long distance_cm = 0;
bool isLightOn = false;
int currentServoAngle = 0;

unsigned long lastSensorRead = 0;
const long sensorInterval = 5000; // 5 detik

// ================== FUNCTION PROTOTYPES (MENCEGAH ERROR KOMPILASI) ==================
void connectWiFiOnce();
void connectMQTTOnce();
void mqttCallback(char* topic, byte* payload, unsigned int length);
long measureDistance();
void readAndPublishSensors();
void updateLCD(); 

// ================== FUNCTIONS ==================

void connectWiFiOnce() {
    WiFi.begin(ssid, password);
    lcd.clear();
    lcd.print("WiFi Connecting");
    Serial.print("Connecting to WiFi ");

    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 25000) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi Connected!");
        lcd.clear();
        lcd.print("WiFi Connected");
        lcd.setCursor(0, 1);
        lcd.print(WiFi.localIP());
        delay(1500);
    } else {
        Serial.println("\nWiFi Failed!");
        lcd.clear();
        lcd.print("WiFi Failed");
        delay(5000);
    }
}

void connectMQTTOnce() {
    client.setServer(mqtt_broker, mqtt_port);
    lcd.clear();
    lcd.print("MQTT Connecting");

    if (client.connect(client_id)) {
        Serial.println("MQTT Connected!");
        lcd.clear();
        lcd.print("MQTT Connected");
        client.subscribe(LIGHT_CONTROL_TOPIC);
        client.subscribe(DOOR_CONTROL_TOPIC);
        delay(1500);
    } else {
        Serial.print("MQTT Failed! RC: ");
        Serial.println(client.state());
        lcd.clear();
        lcd.print("MQTT Failed!");
        delay(5000);
    }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String t = topic;
    String msg;
    for (int i = 0; i < length; i++) msg += (char)payload[i];
    msg.trim();

    Serial.println("Message [" + t + "] : " + msg);

    if (t == LIGHT_CONTROL_TOPIC) {
        if (msg == "ON") {
            digitalWrite(RELAY_PIN, LOW); // Aktifkan relay (Active-LOW)
            isLightOn = true;
        } else if (msg == "OFF") {
            digitalWrite(RELAY_PIN, HIGH); // Matikan relay
            isLightOn = false;
        }
    } 
    else if (t == DOOR_CONTROL_TOPIC) {
        int angle = msg.toInt();
        if (angle >= 0 && angle <= 180) {
            currentServoAngle = angle;
            doorServo.write(angle);
        }
    }
    updateLCD(); 
}

long measureDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    long duration = pulseIn(ECHO_PIN, HIGH, 30000);
    if (duration == 0) return -1;
    return duration * 0.034 / 2; // convert to cm
}

void readAndPublishSensors() {
    humidity = dht.readHumidity();
    temperature_C = dht.readTemperature();

    // Check DHT sensor failure (akan berhasil jika pin 25 benar)
    if (isnan(humidity) || isnan(temperature_C)) {
        Serial.println("Failed to read from DHT sensor!");
        temperature_C = 0.0; // Set ke 0 agar tidak T:nanC
        humidity = 0.0;
    } else {
        String payload = String(temperature_C, 1) + "," + String(humidity, 0);
        if (client.connected()) {
            client.publish(TEMP_HUM_TOPIC, payload.c_str());
            Serial.println("Temp/Hum sent: " + payload);
        }
    }

    distance_cm = measureDistance();
    if (distance_cm > 0 && distance_cm < 400) {
        String payload = String(distance_cm);
        if (client.connected()) {
            client.publish(DISTANCE_TOPIC, payload.c_str());
            Serial.println("Distance sent: " + payload);
        }
    }
}

void updateLCD() {
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(temperature_C, 1);
    lcd.print(" H:");
    lcd.print((int)humidity);
    lcd.print("%  "); 

    lcd.setCursor(0, 1);
    lcd.print(isLightOn ? "L:ON " : "L:OFF");
    lcd.print(" P:");
    lcd.print(currentServoAngle);
    lcd.print(" D:"); 
    lcd.print(distance_cm);
    lcd.print("cm ");
}

void setup() {
    Serial.begin(115200);
    
    Wire.begin(SDA_PIN, SCL_PIN); 
    lcd.init();
    lcd.backlight();

    dht.begin();
    doorServo.attach(SERVO_PIN); 
    doorServo.write(0); 

    pinMode(RELAY_PIN, OUTPUT); 
    digitalWrite(RELAY_PIN, HIGH); 
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    lcd.clear();
    lcd.print("Smart Home Init");
    delay(1000);

    connectWiFiOnce();
    client.setCallback(mqttCallback);
    connectMQTTOnce();
    updateLCD(); 
}

void loop() {
    client.loop();

    unsigned long now = millis();
    
    if (now - lastSensorRead >= sensorInterval && client.connected()) {
        lastSensorRead = now;
        readAndPublishSensors();
        updateLCD(); 
    }
    
    if (WiFi.status() != WL_CONNECTED || !client.connected()) {
        lcd.setCursor(0, 0);
        lcd.print("STATUS: DISCONNECTED  ");
        lcd.setCursor(0, 1);
        lcd.print("Check Wiring/Router! ");
    }
    
    delay(10);
}
