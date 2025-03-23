// Ideas: absolute acceleration below value of 1

#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <math.h>
#include <ESP32Servo.h>


const char* ssid = "Parachute_ESP32_Server";
const char* password = "12345678";

Servo mortarServo;  // Create servo object
int mortarServoPin = 18;  // PWM-capable pin

Servo payloadServo;  // Create servo object
int payloadServoPin = 19;  // PWM-capable pin

WebServer server(80);
WebSocketsServer webSocket(81);
Adafruit_MPU6050 mpu;

unsigned long lastUpdateTime = 0;
const int updateInterval = 10;

bool calibrationDone = false;
float ax_offset = 0, ay_offset = 0, az_offset = 0;
float ax_sum = 0, ay_sum = 0, az_sum = 0;

bool fallDetected = false;
bool parachuteDeployed = false;
String dataLog = "Time,AX,AY,AZ,|A|,FallDetected\n";  // Initialize CSV header

void calibrateSensors();
void sendSensorData(unsigned long currentMillis);

void setupWiFi() {
    WiFi.softAP(ssid, password);
    Serial.println("Access Point Started");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());
}

void handleRoot() {

//      File file = SPIFFS.open("/index.html", "r");
//    if (!file) {
//        server.send(500, "text/plain", "Failed to load index.html");
//        return;
//    }
//    server.streamFile(file, "text/html");
//    file.close();

    
    String htmlContent = "<!DOCTYPE html><html lang=\"en\"><head>"
    "<meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
    "<title>MPU6050 Real-Time Graph</title>"
    "<script src=\"/chart.js\"></script>"  // Load Chart.js from SPIFFS
    "<style>"
    "body { font-family: Arial, sans-serif; text-align: center; margin: 0; max-height: 100vh; display: flex; justify-content: center; align-items: center; flex-direction: column; }"
    "h2 { margin: 20px 0; }"
    "canvas { max-width: 100%; max-height: 60vh; margin-bottom: 20px; }"
    ".alert { background-color: red; color: white; padding: 10px; margin-top: 10px; display: none; }"
    ".button-container { display: flex; justify-content: center; align-items: center; margin-top: 20px; }"
    "button { margin: 10px; padding: 10px; }"
    "#fallAlert { background-color: white; color: black; border: 1px solid black; }"
    "#fallAlert.active { background-color: red; color: white; }"
    "</style></head>"
    "<body><h2>Real-Time Acceleration Data</h2><canvas id=\"accelChart\"></canvas>"
    "<div class=\"button-container\">"
    "<button id=\"resetFall\">Fall Reset</button>"
    "<button id=\"fallAlert\">Fall Detected</button>"
    "<button id=\"deployParachute\">Deploy Parachute</button>"
    "<button id=\"downloadData\">Download Data</button>"
    "</div>"
    "<script>"
    "document.addEventListener('DOMContentLoaded', (event) => {"
    "let ws;"
    "const connectWebSocket = () => {"
    "ws = new WebSocket('ws://' + location.hostname + ':81/');"
    "ws.onopen = () => { console.log('WebSocket connected'); ws.send('keepAlive'); setInterval(() => ws.send('keepAlive'), 30000); };"
    "ws.onmessage = function(event) { const data = JSON.parse(event.data);"
    "accelDataX.push(data.ax); accelDataY.push(data.ay); accelDataZ.push(data.az); accelDataA.push(data.aa);"
    "labels.push(''); if (accelDataX.length > 300) { accelDataX.shift(); accelDataY.shift(); accelDataZ.shift(); accelDataA.shift(); labels.shift(); }"
    "accelChart.update(); if (data.fall) { document.getElementById('fallAlert').classList.add('active'); }"
    "else { document.getElementById('fallAlert').classList.remove('active'); } };"
    "ws.onclose = () => { console.log('WebSocket disconnected, attempting to reconnect...'); setTimeout(connectWebSocket, 1000); };"
    "ws.onerror = (error) => { console.log('WebSocket Error: ', error); ws.close(); };"
    "};"
    "connectWebSocket();"
    "let labels = Array(300).fill('');"
    "let accelDataX = Array(300).fill(0), accelDataY = Array(300).fill(0), accelDataZ = Array(300).fill(0), accelDataA = Array(300).fill(0);"
    "const ctx = document.getElementById('accelChart').getContext('2d');"
    "const accelChart = new Chart(ctx, { type: 'line', data: { labels: labels, datasets: ["
    "{ label: 'X Acceleration', data: accelDataX, borderColor: 'red', fill: false },"
    "{ label: 'Y Acceleration', data: accelDataY, borderColor: 'green', fill: false },"
    "{ label: 'Z Acceleration', data: accelDataZ, borderColor: 'blue', fill: false },"
    "{ label: 'Absolute Acceleration', data: accelDataA, borderColor: 'black', fill: false }] }, options: { animation: false, "
    "responsive: true, scales: { x: { display: false }, y: { beginAtZero: false, suggestedMin: -10, suggestedMax: 10 }}}});"
    "document.getElementById('resetFall').addEventListener('click', () => { ws.send('resetFall'); });"
    "document.getElementById('deployParachute').addEventListener('click', () => { ws.send('deployParachute'); });"
    "document.getElementById('downloadData').addEventListener('click', () => { window.location.href = '/downloadData'; });"  // Trigger download
    "});"
    "</script></body></html>";

    server.send(200, "text/html", htmlContent);
}

void handleChartJS() {
    File file = SPIFFS.open("/chart.js", "r");
    if (!file) {
        Serial.println("chart.js file not found!");
        server.send(404, "text/plain", "chart.js file not found");
        return;
    }
    server.streamFile(file, "application/javascript");
    file.close();
}

void handleDownloadData() {
    File file = SPIFFS.open("/dataLog.csv", "r");
    if (!file) {
        server.send(404, "text/plain", "No data file found");
        return;
    }

    // Stream file to client
    server.streamFile(file, "text/csv");
    file.close();

    // Delete the file after serving it
    SPIFFS.remove("/dataLog.csv");

    // Recreate the file to keep logging enabled after clearing
    file = SPIFFS.open("/dataLog.csv", "w");
    if (file) {
        file.print("Time,AX,AY,AZ,|A|,FallDetected\n");  // Add CSV header
        file.close();
    }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    if (type == WStype_CONNECTED) {
        Serial.println("Client connected.");
        unsigned long esp32Time = millis();
        String timeSyncMessage = "{\"timeSync\": " + String(esp32Time) + "}";
        webSocket.sendTXT(num, timeSyncMessage);
    } else if (type == WStype_TEXT) {
        String message = String((char *)payload);
        if (message == "resetFall") {
            fallDetected = false;
            parachuteDeployed = false;
            Serial.println("Fall Status Reset");
        } else if (message == "deployParachute") {
            fallDetected = true;
            parachuteDeployed = false;
            Serial.println("Parachute Deployed Manually");
        }

//        else if message is deployPayload
    }
}

void setup() {
    Serial.begin(115200);
    setupWiFi();

    if (!SPIFFS.begin(true)) {
        Serial.println("Failed to mount SPIFFS");
        return;
    }
    Serial.println("SPIFFS mounted successfully");

    // Delete the previous log file at startup
    if (SPIFFS.exists("/dataLog.csv")) {
        SPIFFS.remove("/dataLog.csv");
        Serial.println("Old data log deleted.");
    }

    // Create a new data log file and add headers
    File file = SPIFFS.open("/dataLog.csv", "w");
    if (file) {
        file.println("Time,AX,AY,AZ,|A|,FallDetected");  // Write the header
        file.close();
        Serial.println("New data log initialized with headers.");
    } else {
        Serial.println("Failed to create new data log.");
    }

    server.on("/", handleRoot);
    server.on("/chart.js", HTTP_GET, handleChartJS);
    server.on("/downloadData", HTTP_GET, handleDownloadData);
    server.begin();

    if (!mpu.begin()) {
        Serial.println("MPU6050 initialization failed!");
        return;
    }
   
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);    // Low-pass filter

    calibrateSensors();

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    mortarServo.attach(mortarServoPin); // Attach mortar servo to the pin
    mortarServo.write(65);    // Move to 0°

    payloadServo.attach(payloadServoPin); // Attach payload servo to the pin
    payloadServo.write(0);
    
}

void loop() {
    server.handleClient();
    webSocket.loop();
    unsigned long currentMillis = millis();
    if (currentMillis - lastUpdateTime >= updateInterval) {
        lastUpdateTime = currentMillis;
        sendSensorData(currentMillis);
    }
    if (fallDetected == true && parachuteDeployed == false){
      mortarServo.write(115);   // Move to 90°
      parachuteDeployed = true;
    }
}

void calibrateSensors() {

    const int calibrationSamples = 100;

    for (int i = 0; i < calibrationSamples; i++) {
        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);
      
        ax_sum += a.acceleration.x;
        ay_sum += a.acceleration.y;
        az_sum += a.acceleration.z;

        delay(10);
    }

   // Compute averages
        ax_offset = ax_sum / calibrationSamples;
        ay_offset = ay_sum / calibrationSamples;
        az_offset = az_sum / calibrationSamples;
        calibrationDone = true;
        Serial.println("Calibration complete");
}

void sendSensorData(unsigned long currentMillis) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    float ax_corrected = a.acceleration.x - ax_offset;
    float ay_corrected = a.acceleration.y - ay_offset;
    float az_corrected = -1* (a.acceleration.z - az_offset - 9.81);

    float a_absolute = sqrt(ax_corrected**2 + ay_corrected**2 + az_corrected**2);

    // Fall detection logic using absolute accleration less than 1m/s^2
    if (a_absolute <= 1 && parachuteDeployed == false) {
          fallDetected = true;
          mortarServo.write(115);   // Move to 90°
          Serial.println("Fall Detected and Parachute Deployed");
          parachuteDeployed = true;
    }

    float seconds = currentMillis / 1000.0;
    String dataEntry = String(seconds) + "," + String(ax_corrected) + "," + String(ay_corrected) + "," + String(az_corrected) + "," + String(a_absolute) + "," + String(fallDetected) + "\n";

    // Save to SPIFFS
    File file = SPIFFS.open("/dataLog.csv", "a");
    if (file) {
        file.print(dataEntry);
        file.close();
    }

    // WebSocket broadcast
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["time"] = seconds;
    jsonDoc["ax"] = ax_corrected;
    jsonDoc["ay"] = ay_corrected;
    jsonDoc["az"] = az_corrected;
    jsonDoc["aa"] = a_absolute;
    jsonDoc["fall"] = fallDetected;
    String jsonString;
    serializeJson(jsonDoc, jsonString);
    webSocket.broadcastTXT(jsonString);
}