/*
 * ============================================================
 *  ESP32 Temperature-Controlled Fan System
 * ============================================================
 *  
 *  Reads a DHT22 temperature/humidity sensor, controls a 
 *  single relay (ON/OFF) based on configurable thresholds,
 *  and serves a real-time web dashboard over WiFi (STA mode).
 *
 *  Hardware:
 *    - ESP32 DevKit
 *    - DHT22 sensor on GPIO4
 *    - Relay module on GPIO16 (active-LOW)
 *    - Onboard LED on GPIO2 (status indicator)
 *
 *  Communication:
 *    - HTTP server on port 80  (serves dashboard HTML)
 *    - WebSocket server on port 81 (real-time data + commands)
 *
 *  Libraries required:
 *    - WiFi.h            (built-in ESP32)
 *    - WebServer.h       (built-in ESP32)
 *    - WebSocketsServer.h (Links2004/arduinoWebSockets)
 *    - DHT.h             (Adafruit DHT sensor library)
 *    - ArduinoJson.h     (v6, bblanchon/ArduinoJson)
 *
 * ============================================================
 */

// ============================================
// Includes
// ============================================
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <DHT.h>
#include <ArduinoJson.h>

#include "config.h"
#include "web_page.h"

// ============================================
// Object Instances
// ============================================
DHT dht(DHT_PIN, DHT_TYPE);
WebServer server(80);
WebSocketsServer webSocket(81);

// ============================================
// Global Variables
// ============================================
float currentTemp = 0.0;
float currentHumidity = 0.0;
bool fanOn = false;
bool autoMode = true;
float threshold = DEFAULT_THRESHOLD;
float hysteresis = DEFAULT_HYSTERESIS;
unsigned long lastSensorRead = 0;
unsigned long lastBroadcast = 0;
unsigned long startTime = 0;

// Cycle protection, alarm, and physical switch variables
unsigned long lastStateChangeTime = 0; // Tracks when the fan was toggled ON or OFF
bool contactorFault = false;           // Mismatch between commanded state and feedback
bool tempAlarm = false;                // High temperature alarm state
String physicalSwitchState = "OFF";    // "AUTO", "OFF", or "ON"
bool clientAuthenticated[10] = {false}; // Tracks settings auth state per WebSocket client

// ============================================
// Function Prototypes
// ============================================
void setFan(bool on);
void handleRoot();
void broadcastStatus();
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
void readPhysicalSwitch();
String getLockType(unsigned long &remainingSec);

// ============================================
// Setup
// ============================================
void setup() {
    // --- Serial Debug ---
    Serial.begin(115200);
    Serial.println();
    Serial.println("========================================");
    Serial.println("  ESP32 Temperature-Controlled Fan");
    Serial.println("========================================");

    // --- Pin Configuration ---
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, RELAY_OFF);  // Fan starts OFF

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // Physical inputs
    pinMode(SWITCH_AUTO_PIN, INPUT_PULLUP);
    pinMode(SWITCH_ON_PIN, INPUT_PULLUP);
    pinMode(FEEDBACK_PIN, INPUT_PULLUP);
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    // --- DHT Sensor Init ---
    dht.begin();
    Serial.println("[INIT] DHT22 sensor initialized.");

    // --- WiFi Connection (Station Mode) ---
    Serial.print("[WIFI] Connecting to ");
    Serial.print(WIFI_SSID);
    Serial.print("...");

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long wifiStart = millis();
    bool ledState = false;

    while (WiFi.status() != WL_CONNECTED) {
        // Timeout check
        if (millis() - wifiStart > WIFI_TIMEOUT) {
            Serial.println();
            Serial.println("[WIFI] Connection timeout! Restarting...");
            ESP.restart();
        }

        // Blink LED while connecting
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState ? HIGH : LOW);

        delay(500);
        Serial.print(".");
    }

    // WiFi connected
    Serial.println();
    Serial.println("[WIFI] Connected!");
    Serial.print("[WIFI] IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("[WIFI] Signal Strength (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");

    // LED solid ON to indicate connection
    digitalWrite(LED_PIN, HIGH);

    // --- HTTP Server Setup ---
    server.on("/", handleRoot);
    server.begin();
    Serial.println("[HTTP] Web server started on port 80.");

    // --- WebSocket Server Setup ---
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    Serial.println("[WS]   WebSocket server started on port 81.");

    // --- Record Start Time ---
    startTime = millis();
    lastStateChangeTime = millis(); // Set initial cycle protection reference

    // Read initial physical switch state
    readPhysicalSwitch();

    Serial.println("========================================");
    Serial.println("  System ready! Open the dashboard at:");
    Serial.print("  http://");
    Serial.println(WiFi.localIP());
    Serial.println("========================================");
}

// ============================================
// Main Loop
// ============================================
void loop() {
    // Handle HTTP and WebSocket clients
    server.handleClient();
    webSocket.loop();

    unsigned long now = millis();

    // --- Read Physical Switch State ---
    readPhysicalSwitch();

    // --- Sensor Reading (every SENSOR_INTERVAL ms) ---
    if (now - lastSensorRead >= SENSOR_INTERVAL) {
        lastSensorRead = now;

        float t = dht.readTemperature();
        float h = dht.readHumidity();

        // Only update if readings are valid
        if (!isnan(t) && !isnan(h)) {
            currentTemp = t;
            currentHumidity = h;

            // --- High Temperature Alarm ---
            if (currentTemp >= ALARM_THRESHOLD) {
                if (!tempAlarm) {
                    tempAlarm = true;
                    Serial.print("[ALARM] High temperature detected: ");
                    Serial.print(currentTemp, 1);
                    Serial.println("°C");
                    broadcastStatus();
                }
            } else if (currentTemp < (ALARM_THRESHOLD - ALARM_HYSTERESIS)) {
                if (tempAlarm) {
                    tempAlarm = false;
                    Serial.println("[ALARM] High temperature alarm cleared.");
                    broadcastStatus();
                }
            }

            // --- Auto Mode: Threshold Logic ---
            if (autoMode) {
                unsigned long elapsed = now - lastStateChangeTime;
                // Turn fan ON if temperature >= threshold and fan is currently OFF
                if (currentTemp >= threshold && !fanOn) {
                    if (elapsed >= MIN_STOP_TIME_MS) {
                        Serial.print("[AUTO] Temp ");
                        Serial.print(currentTemp, 1);
                        Serial.print("°C >= threshold ");
                        Serial.print(threshold, 1);
                        Serial.println("°C → Fan ON");
                        setFan(true);
                    }
                }
                // Turn fan OFF if temperature < (threshold - hysteresis) and fan is ON
                else if (currentTemp < (threshold - hysteresis) && fanOn) {
                    if (elapsed >= MIN_RUN_TIME_MS) {
                        Serial.print("[AUTO] Temp ");
                        Serial.print(currentTemp, 1);
                        Serial.print("°C < ");
                        Serial.print(threshold - hysteresis, 1);
                        Serial.println("°C → Fan OFF");
                        setFan(false);
                    }
                }
            }
        } else {
            Serial.println("[SENSOR] Warning: Failed to read from DHT22!");
        }
    }

    // --- Contactor Feedback Check ---
    if (now - lastStateChangeTime >= FEEDBACK_DELAY_MS) {
        bool feedbackClosed = (digitalRead(FEEDBACK_PIN) == LOW);
        if (fanOn && !feedbackClosed) {
            if (!contactorFault) {
                contactorFault = true;
                Serial.println("[FAULT] Contactor failed to pull in! (Feedback HIGH when commanded ON)");
                broadcastStatus();
            }
        } else if (!fanOn && feedbackClosed) {
            if (!contactorFault) {
                contactorFault = true;
                Serial.println("[FAULT] Contactor stuck closed! (Feedback LOW when commanded OFF)");
                broadcastStatus();
            }
        } else {
            if (contactorFault) {
                contactorFault = false;
                Serial.println("[FAULT] Contactor fault cleared.");
                broadcastStatus();
            }
        }
    }

    // --- Buzzer Control (Pulse on Alarm/Fault) ---
    // Only sound the high-temp alarm if the fan is OFF. Contactor fault always sounds.
    if ((tempAlarm && !fanOn) || contactorFault) {
        // Pulse buzzer: 200ms ON, 800ms OFF
        if ((now % 1000) < 200) {
            digitalWrite(BUZZER_PIN, HIGH);
        } else {
            digitalWrite(BUZZER_PIN, LOW);
        }
    } else {
        digitalWrite(BUZZER_PIN, LOW);
    }

    // --- Broadcast Status (every BROADCAST_INTERVAL ms) ---
    if (now - lastBroadcast >= BROADCAST_INTERVAL) {
        lastBroadcast = now;
        broadcastStatus();
    }
}

// ============================================
// Set Fan State
// ============================================
void setFan(bool on) {
    fanOn = on;
    digitalWrite(RELAY_PIN, on ? RELAY_ON : RELAY_OFF);
    digitalWrite(LED_PIN, on ? HIGH : LOW);
    lastStateChangeTime = millis();  // Reset protection and feedback timers
    Serial.println(on ? "[FAN] Fan: ON" : "[FAN] Fan: OFF");
    broadcastStatus();  // Immediate update to all clients
}

// ============================================
// HTTP: Serve Dashboard
// ============================================
void handleRoot() {
    server.send_P(200, "text/html", DASHBOARD_HTML);
    Serial.println("[HTTP] Dashboard served to client.");
}

// ============================================
// Broadcast Status via WebSocket
// ============================================
void broadcastStatus() {
    unsigned long remainingLock = 0;
    String lockType = getLockType(remainingLock);

    StaticJsonDocument<384> doc;
    doc["temp"] = round(currentTemp * 10.0) / 10.0;
    doc["humidity"] = round(currentHumidity * 10.0) / 10.0;
    doc["fan"] = fanOn;
    doc["mode"] = autoMode ? "auto" : "manual";
    doc["threshold"] = threshold;
    doc["hysteresis"] = hysteresis;
    doc["uptime"] = (millis() - startTime) / 1000;
    doc["rssi"] = WiFi.RSSI();
    
    // New parameters
    doc["switch"] = physicalSwitchState;
    doc["fault"] = contactorFault;
    doc["alarm"] = tempAlarm;
    doc["lock_type"] = lockType;
    doc["lock_time"] = remainingLock;
    doc["alarm_threshold"] = ALARM_THRESHOLD;

    String json;
    serializeJson(doc, json);
    webSocket.broadcastTXT(json);
}

// ============================================
// WebSocket Event Handler
// ============================================
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {

        case WStype_CONNECTED: {
            IPAddress ip = webSocket.remoteIP(num);
            Serial.printf("[WS] Client #%u connected from %s\n", num, ip.toString().c_str());

            // Initialize auth state to locked
            if (num < 10) clientAuthenticated[num] = false;

            // Send current status to the newly connected client
            unsigned long remainingLock = 0;
            String lockType = getLockType(remainingLock);

            StaticJsonDocument<384> doc;
            doc["temp"] = round(currentTemp * 10.0) / 10.0;
            doc["humidity"] = round(currentHumidity * 10.0) / 10.0;
            doc["fan"] = fanOn;
            doc["mode"] = autoMode ? "auto" : "manual";
            doc["threshold"] = threshold;
            doc["hysteresis"] = hysteresis;
            doc["uptime"] = (millis() - startTime) / 1000;
            doc["rssi"] = WiFi.RSSI();

            doc["switch"] = physicalSwitchState;
            doc["fault"] = contactorFault;
            doc["alarm"] = tempAlarm;
            doc["lock_type"] = lockType;
            doc["lock_time"] = remainingLock;
            doc["alarm_threshold"] = ALARM_THRESHOLD;

            String json;
            serializeJson(doc, json);
            webSocket.sendTXT(num, json);
            break;
        }

        case WStype_DISCONNECTED:
            Serial.printf("[WS] Client #%u disconnected.\n", num);
            if (num < 10) clientAuthenticated[num] = false;
            break;

        case WStype_TEXT: {
            Serial.printf("[WS] Client #%u sent: %s\n", num, payload);

            // Parse incoming JSON command
            StaticJsonDocument<256> doc;
            DeserializationError error = deserializeJson(doc, payload, length);

            if (error) {
                Serial.print("[WS] JSON parse error: ");
                Serial.println(error.c_str());
                break;
            }

            const char* action = doc["action"];
            if (!action) {
                Serial.println("[WS] No 'action' field in message.");
                break;
            }

            // --- Handle: Toggle Fan ---
            if (strcmp(action, "toggle") == 0) {
                if (!autoMode) {
                    setFan(!fanOn);
                    Serial.println("[CMD] Fan toggled (manual mode).");
                } else {
                    Serial.println("[CMD] Toggle ignored — auto mode is active.");
                }
            }

            // --- Handle: Set Mode (auto/manual) ---
            else if (strcmp(action, "mode") == 0) {
                bool newMode = doc["value"] | true;
                autoMode = newMode;
                Serial.print("[CMD] Mode set to: ");
                Serial.println(autoMode ? "AUTO" : "MANUAL");
                broadcastStatus();
            }

            // --- Handle: Settings Authentication ---
            else if (strcmp(action, "auth") == 0) {
                const char* pwd = doc["value"];
                bool success = (pwd && strcmp(pwd, SETTINGS_PASSWORD) == 0);
                if (num < 10) clientAuthenticated[num] = success;
                
                // Reply specifically to this client
                StaticJsonDocument<128> resDoc;
                resDoc["action"] = "auth_res";
                resDoc["success"] = success;
                String responseJson;
                serializeJson(resDoc, responseJson);
                webSocket.sendTXT(num, responseJson);
                
                Serial.printf("[CMD] Client #%u authentication: %s\n", num, success ? "SUCCESS" : "FAILED");
            }

            // --- Handle: Set Threshold ---
            else if (strcmp(action, "threshold") == 0) {
                if (num >= 10 || !clientAuthenticated[num]) {
                    // Send auth required warning to this client
                    StaticJsonDocument<128> resDoc;
                    resDoc["action"] = "auth_req";
                    String responseJson;
                    serializeJson(resDoc, responseJson);
                    webSocket.sendTXT(num, responseJson);
                    Serial.printf("[CMD] Blocked unauthorized threshold edit from client #%u\n", num);
                    break;
                }
                float newThreshold = doc["value"] | DEFAULT_THRESHOLD;
                threshold = newThreshold;
                Serial.print("[CMD] Threshold updated to: ");
                Serial.print(threshold, 1);
                Serial.println("°C");
                broadcastStatus();
            }

            // --- Handle: Set Hysteresis ---
            else if (strcmp(action, "hysteresis") == 0) {
                if (num >= 10 || !clientAuthenticated[num]) {
                    // Send auth required warning to this client
                    StaticJsonDocument<128> resDoc;
                    resDoc["action"] = "auth_req";
                    String responseJson;
                    serializeJson(resDoc, responseJson);
                    webSocket.sendTXT(num, responseJson);
                    Serial.printf("[CMD] Blocked unauthorized hysteresis edit from client #%u\n", num);
                    break;
                }
                float newHysteresis = doc["value"] | DEFAULT_HYSTERESIS;
                hysteresis = newHysteresis;
                Serial.print("[CMD] Hysteresis updated to: ");
                Serial.print(hysteresis, 1);
                Serial.println("°C");
                broadcastStatus();
            }

            else {
                Serial.print("[WS] Unknown action: ");
                Serial.println(action);
            }

            break;
        }

        default:
            break;
    }
}

// ============================================
// Read Physical Switch State
// ============================================
void readPhysicalSwitch() {
    bool autoState = (digitalRead(SWITCH_AUTO_PIN) == LOW);
    bool onState = (digitalRead(SWITCH_ON_PIN) == LOW);

    String lastState = physicalSwitchState;

    if (autoState) {
        physicalSwitchState = "AUTO";
        autoMode = true;
    } else if (onState) {
        physicalSwitchState = "ON";
        autoMode = false;
        if (!fanOn) {
            Serial.println("[SWITCH] Physical switch turned ON → Forcing Fan ON");
            setFan(true);
        }
    } else {
        physicalSwitchState = "OFF";
        autoMode = false;
        if (fanOn) {
            Serial.println("[SWITCH] Physical switch turned OFF → Forcing Fan OFF");
            setFan(false);
        }
    }

    if (physicalSwitchState != lastState) {
        Serial.print("[SWITCH] Physical switch state changed to: ");
        Serial.println(physicalSwitchState);
        broadcastStatus();
    }
}

// ============================================
// Get Lock Type and Remaining Lock Duration
// ============================================
String getLockType(unsigned long &remainingSec) {
    if (autoMode) {
        unsigned long elapsed = millis() - lastStateChangeTime;
        if (fanOn) {
            if (elapsed < MIN_RUN_TIME_MS) {
                remainingSec = (MIN_RUN_TIME_MS - elapsed) / 1000;
                return "run";
            }
        } else {
            if (elapsed < MIN_STOP_TIME_MS) {
                remainingSec = (MIN_STOP_TIME_MS - elapsed) / 1000;
                return "stop";
            }
        }
    }
    remainingSec = 0;
    return "none";
}
