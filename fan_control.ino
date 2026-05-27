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
#include <ESPmDNS.h>
#include <DNSServer.h>
#include <Preferences.h>
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
DNSServer dnsServer;
Preferences prefs;

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
bool provisioningMode = false;
String wifiSsid = "";
String wifiPassword = "";
unsigned long lastWiFiReconnectAttempt = 0;
bool mdnsStarted = false;

// --- Runtime tracking & forced-rest (cooldown) state ---
unsigned long fanOnSince = 0;          // millis() when current continuous ON run started (0 if OFF)
unsigned long currentRunMs = 0;        // Length of current continuous ON run (live)
uint64_t totalRunMs = 0;               // Total accumulated ON-time, persisted across reboot
unsigned long fanCycleCount = 0;       // Number of OFF -> ON transitions since boot

bool cooldownActive = false;           // True while motor is in forced rest
unsigned long cooldownStart = 0;       // millis() when cooldown started

// Runtime-configurable rest-cycle parameters (defaults from config.h)
unsigned long maxContRunMs = DEFAULT_MAX_CONT_RUN_MS;
unsigned long cooldownMs   = DEFAULT_COOLDOWN_MS;

// --- Sensor health / fail-safe ---
unsigned int consecutiveSensorFailures = 0;
unsigned long lastValidSensorRead = 0;
bool sensorFailSafeActive = false;
String sensorStatus = "unknown";       // "unknown", "ok", "stale", "failed"

// --- Debounced physical inputs ---
struct DebouncedInput {
    uint8_t pin;
    bool stableLow;
    bool rawLow;
    unsigned long lastRawChange;
};

DebouncedInput switchAutoInput = {SWITCH_AUTO_PIN, false, false, 0};
DebouncedInput switchOnInput   = {SWITCH_ON_PIN, false, false, 0};
DebouncedInput feedbackInput   = {FEEDBACK_PIN, false, false, 0};

// --- Controller state / dashboard health ---
enum ControllerState {
    STATE_OFF,
    STATE_AUTO_WAIT,
    STATE_RUNNING,
    STATE_MIN_RUN_LOCK,
    STATE_MIN_STOP_LOCK,
    STATE_COOLDOWN,
    STATE_FAULT
};

ControllerState controllerState = STATE_OFF;
String activeLockReason = "none";
String lastNotificationEvent = "boot";

// --- Non-blocking buzzer event pattern ---
bool buzzerEventActive = false;
unsigned int buzzerEventRemainingToggles = 0;
unsigned long buzzerEventNextToggle = 0;
unsigned int buzzerEventOnMs = 80;
unsigned int buzzerEventOffMs = 120;
bool buzzerEventOn = false;

// ============================================
// Function Prototypes
// ============================================
void setFan(bool on);
void handleRoot();
void broadcastStatus();
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
void readPhysicalSwitch();
String getLockType(unsigned long &remainingSec);
void updateRuntimeStats();
void checkContinuousRunLimit();
void endCooldown(const char *reason);
void populateStatusDoc(JsonDocument &doc);
void loadPersistentSettings();
void savePersistentSettings();
void saveRuntimeStats();
void loadWiFiCredentials();
bool connectWiFi();
void startProvisioningMode();
void handleProvisioningRoot();
void handleProvisioningSave();
void handleNotFound();
void maintainWiFi();
void startMDNS();
void updateSensorStatus();
void handleSensorFailure();
bool updateDebouncedInput(DebouncedInput &input, unsigned long debounceMs);
void updateControllerState();
const char* controllerStateName();
String wifiQuality();
bool isClientAuthed(uint8_t num);
void sendAuthRequired(uint8_t num);
void sendCommandError(uint8_t num, const char *message);
float clampFloat(float value, float minValue, float maxValue);
unsigned long clampULong(unsigned long value, unsigned long minValue, unsigned long maxValue);
void triggerBuzzerEvent(unsigned int beeps, unsigned int onMs, unsigned int offMs);
void updateBuzzer();
void emitEvent(const char *eventName);

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
    updateDebouncedInput(switchAutoInput, 0);
    updateDebouncedInput(switchOnInput, 0);
    updateDebouncedInput(feedbackInput, 0);

    // --- DHT Sensor Init ---
    dht.begin();
    Serial.println("[INIT] DHT22 sensor initialized.");
    lastValidSensorRead = 0;

    // --- Load persisted settings / WiFi credentials ---
    loadPersistentSettings();
    loadWiFiCredentials();

    // --- WiFi Connection (Station Mode) ---
    bool wifiConnected = connectWiFi();
    if (!wifiConnected) {
        startProvisioningMode();
    } else {
        startMDNS();
        digitalWrite(LED_PIN, HIGH);
    }

    // --- HTTP Server Setup ---
    if (provisioningMode) {
        server.on("/", HTTP_GET, handleProvisioningRoot);
        server.on("/save", HTTP_POST, handleProvisioningSave);
        server.onNotFound(handleNotFound);
    } else {
        server.on("/", handleRoot);
    }
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
    Serial.println(provisioningMode ? WiFi.softAPIP() : WiFi.localIP());
    if (!provisioningMode && mdnsStarted) {
        Serial.print("  http://");
        Serial.print(WIFI_HOSTNAME);
        Serial.println(".local");
    }
    Serial.println("========================================");
}

// ============================================
// Main Loop
// ============================================
void loop() {
    // Handle HTTP and WebSocket clients
    server.handleClient();
    webSocket.loop();
    if (provisioningMode) {
        dnsServer.processNextRequest();
    } else {
        maintainWiFi();
    }

    unsigned long now = millis();

    updateDebouncedInput(switchAutoInput, SWITCH_DEBOUNCE_MS);
    updateDebouncedInput(switchOnInput, SWITCH_DEBOUNCE_MS);
    updateDebouncedInput(feedbackInput, FEEDBACK_DEBOUNCE_MS);

    // --- Read Physical Switch State ---
    readPhysicalSwitch();

    // --- Update runtime stats every loop ---
    updateRuntimeStats();

    // --- Enforce 12h continuous-run limit (forced cooldown) ---
    checkContinuousRunLimit();
    updateSensorStatus();
    updateControllerState();

    // --- Clear cooldown when its duration has elapsed ---
    if (cooldownActive && (now - cooldownStart) >= cooldownMs) {
        endCooldown("timer elapsed");
    }

    // --- Sensor Reading (every SENSOR_INTERVAL ms) ---
    if (now - lastSensorRead >= SENSOR_INTERVAL) {
        lastSensorRead = now;

        float t = dht.readTemperature();
        float h = dht.readHumidity();

        // Only update if readings are valid
        if (!isnan(t) && !isnan(h)) {
            currentTemp = t;
            currentHumidity = h;
            consecutiveSensorFailures = 0;
            lastValidSensorRead = now;
            sensorFailSafeActive = false;
            updateSensorStatus();

            // --- High Temperature Alarm ---
            if (currentTemp >= ALARM_THRESHOLD) {
                if (!tempAlarm) {
                    tempAlarm = true;
                    Serial.print("[ALARM] High temperature detected: ");
                    Serial.print(currentTemp, 1);
                    Serial.println("°C");
                    emitEvent("high_temperature_alarm");
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
                    // Cooldown lock blocks auto-ON
                    if (cooldownActive) {
                        // Silently respect cooldown; status broadcasts the lock.
                    } else if (elapsed >= MIN_STOP_TIME_MS) {
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
            handleSensorFailure();
        }
    }

    // --- Contactor Feedback Check ---
    if (now - lastStateChangeTime >= FEEDBACK_DELAY_MS) {
        bool feedbackClosed = feedbackInput.stableLow;
        if (fanOn && !feedbackClosed) {
            if (!contactorFault) {
                contactorFault = true;
                Serial.println("[FAULT] Contactor failed to pull in! (Feedback HIGH when commanded ON)");
                emitEvent("contactor_failed_to_pull_in");
                broadcastStatus();
            }
        } else if (!fanOn && feedbackClosed) {
            if (!contactorFault) {
                contactorFault = true;
                Serial.println("[FAULT] Contactor stuck closed! (Feedback LOW when commanded OFF)");
                emitEvent("contactor_stuck_closed");
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

    updateBuzzer();

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
    // If state isn't actually changing, still refresh the relay pin but skip
    // runtime bookkeeping to avoid double counting cycles.
    bool stateChanged = (on != fanOn);

    fanOn = on;
    digitalWrite(RELAY_PIN, on ? RELAY_ON : RELAY_OFF);
    digitalWrite(LED_PIN, on ? HIGH : LOW);
    lastStateChangeTime = millis();  // Reset protection and feedback timers

    if (stateChanged) {
        if (on) {
            // OFF -> ON: start a new continuous-run timer
            fanOnSince = millis();
            currentRunMs = 0;
            fanCycleCount++;
        } else {
            // ON -> OFF: bank the elapsed run-time then reset the timer
            if (fanOnSince > 0) {
                totalRunMs += (millis() - fanOnSince);
            }
            fanOnSince = 0;
            currentRunMs = 0;
            saveRuntimeStats();
        }
    }

    Serial.println(on ? "[FAN] Fan: ON" : "[FAN] Fan: OFF");
    broadcastStatus();  // Immediate update to all clients
}

// ============================================
// Update Runtime Stats (called every loop)
// ============================================
void updateRuntimeStats() {
    if (fanOn && fanOnSince > 0) {
        currentRunMs = millis() - fanOnSince;
    } else {
        currentRunMs = 0;
    }
}

// ============================================
// Continuous-Run Limit Enforcement
// ============================================
void checkContinuousRunLimit() {
    if (!fanOn || cooldownActive) return;
    if (fanOnSince == 0) return;

    if ((millis() - fanOnSince) >= maxContRunMs) {
        Serial.print("[REST] Fan ran continuously for ");
        Serial.print((millis() - fanOnSince) / 60000UL);
        Serial.print(" min → forcing OFF for cooldown (");
        Serial.print(cooldownMs / 60000UL);
        Serial.println(" min).");

        cooldownActive = true;
        cooldownStart = millis();

        // Force the fan OFF unconditionally
        setFan(false);

        emitEvent("cooldown_started");
        triggerBuzzerEvent(2, 120, 120);

        broadcastStatus();
    }
}

// ============================================
// End Cooldown
// ============================================
void endCooldown(const char *reason) {
    cooldownActive = false;
    cooldownStart = 0;
    Serial.print("[REST] Cooldown ended (");
    Serial.print(reason);
    Serial.println("). Normal operation resumed.");
    emitEvent("cooldown_ended");
    broadcastStatus();
}

// ============================================
// HTTP: Serve Dashboard
// ============================================
void handleRoot() {
    server.send_P(200, "text/html", DASHBOARD_HTML);
    Serial.println("[HTTP] Dashboard served to client.");
}

// ============================================
// Populate Status JSON (shared helper)
// ============================================
void populateStatusDoc(JsonDocument &doc) {
    unsigned long remainingLock = 0;
    String lockType = getLockType(remainingLock);

    doc["temp"] = round(currentTemp * 10.0) / 10.0;
    doc["humidity"] = round(currentHumidity * 10.0) / 10.0;
    doc["fan"] = fanOn;
    doc["mode"] = autoMode ? "auto" : "manual";
    doc["threshold"] = threshold;
    doc["hysteresis"] = hysteresis;
    doc["uptime"] = (millis() - startTime) / 1000;
    doc["rssi"] = WiFi.RSSI();
    doc["wifi_quality"] = wifiQuality();
    doc["hostname"] = WIFI_HOSTNAME;
    doc["provisioning_mode"] = provisioningMode;
    doc["firmware_version"] = "2.0.0";

    doc["switch"] = physicalSwitchState;
    doc["fault"] = contactorFault;
    doc["alarm"] = tempAlarm;
    doc["lock_type"] = lockType;
    doc["lock_time"] = remainingLock;
    doc["active_lock_reason"] = activeLockReason;
    doc["controller_state"] = controllerStateName();
    doc["feedback_closed"] = feedbackInput.stableLow;
    doc["alarm_threshold"] = ALARM_THRESHOLD;

    doc["sensor_status"] = sensorStatus;
    doc["sensor_failures"] = consecutiveSensorFailures;
    doc["sensor_fail_safe_active"] = sensorFailSafeActive;
    doc["sensor_last_update_sec"] = (lastValidSensorRead > 0) ? ((millis() - lastValidSensorRead) / 1000UL) : 0;
    doc["last_event"] = lastNotificationEvent;

    // --- Runtime tracking ---
    // Current continuous-run time and total accumulated time include the in-
    // progress run so the dashboard updates live without waiting for an OFF.
    uint64_t liveTotal = totalRunMs + (fanOn && fanOnSince > 0 ? (uint64_t)(millis() - fanOnSince) : 0);
    doc["current_run_sec"] = currentRunMs / 1000UL;
    doc["total_run_sec"]   = (uint64_t)(liveTotal / 1000ULL);
    doc["cycle_count"]     = fanCycleCount;

    // --- Forced rest (cooldown) ---
    doc["cooldown_active"] = cooldownActive;
    unsigned long cdRemain = 0;
    if (cooldownActive) {
        unsigned long elapsed = millis() - cooldownStart;
        cdRemain = (elapsed < cooldownMs) ? (cooldownMs - elapsed) / 1000UL : 0;
    }
    doc["cooldown_remaining_sec"] = cdRemain;
    doc["max_cont_run_sec"] = maxContRunMs / 1000UL;
    doc["cooldown_sec"]     = cooldownMs / 1000UL;
    // Bounds so the UI can clamp slider values without hard-coding them.
    doc["max_cont_run_min_sec"] = MIN_MAX_CONT_RUN_MS / 1000UL;
    doc["max_cont_run_max_sec"] = MAX_MAX_CONT_RUN_MS / 1000UL;
    doc["cooldown_min_sec"]     = MIN_COOLDOWN_MS / 1000UL;
    doc["cooldown_max_sec"]     = MAX_COOLDOWN_MS / 1000UL;
}

// ============================================
// Broadcast Status via WebSocket
// ============================================
void broadcastStatus() {
    StaticJsonDocument<1536> doc;
    populateStatusDoc(doc);
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
            StaticJsonDocument<1536> doc;
            populateStatusDoc(doc);
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
                if (cooldownActive && !fanOn) {
                    // During cooldown, only OFF -> ON is blocked; we never
                    // need to block ON -> OFF since the fan is already OFF.
                    Serial.println("[CMD] Toggle ignored — motor in cooldown.");
                } else if (!autoMode) {
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
                if (!isClientAuthed(num)) {
                    sendAuthRequired(num);
                    Serial.printf("[CMD] Blocked unauthorized threshold edit from client #%u\n", num);
                    break;
                }
                float newThreshold = doc["value"] | DEFAULT_THRESHOLD;
                threshold = clampFloat(newThreshold, MIN_THRESHOLD, MAX_THRESHOLD);
                Serial.print("[CMD] Threshold updated to: ");
                Serial.print(threshold, 1);
                Serial.println("°C");
                savePersistentSettings();
                broadcastStatus();
            }

            // --- Handle: Set Hysteresis ---
            else if (strcmp(action, "hysteresis") == 0) {
                if (!isClientAuthed(num)) {
                    sendAuthRequired(num);
                    Serial.printf("[CMD] Blocked unauthorized hysteresis edit from client #%u\n", num);
                    break;
                }
                float newHysteresis = doc["value"] | DEFAULT_HYSTERESIS;
                float maxAllowedHysteresis = MAX_HYSTERESIS;
                if ((threshold - MIN_THRESHOLD + MIN_HYSTERESIS) < maxAllowedHysteresis) {
                    maxAllowedHysteresis = threshold - MIN_THRESHOLD + MIN_HYSTERESIS;
                }
                hysteresis = clampFloat(newHysteresis, MIN_HYSTERESIS, maxAllowedHysteresis);
                Serial.print("[CMD] Hysteresis updated to: ");
                Serial.print(hysteresis, 1);
                Serial.println("°C");
                savePersistentSettings();
                broadcastStatus();
            }

            // --- Handle: End Cooldown (password-protected emergency override) ---
            else if (strcmp(action, "end_cooldown") == 0) {
                if (!isClientAuthed(num)) {
                    sendAuthRequired(num);
                    Serial.printf("[CMD] Blocked unauthorized cooldown override from client #%u\n", num);
                    break;
                }
                if (cooldownActive) {
                    Serial.printf("[CMD] Cooldown manually ended by client #%u.\n", num);
                    endCooldown("manual override");
                } else {
                    Serial.println("[CMD] end_cooldown ignored — no cooldown is active.");
                }
            }

            // --- Handle: Set Max Continuous Run Time (minutes) ---
            else if (strcmp(action, "max_run") == 0) {
                if (!isClientAuthed(num)) {
                    sendAuthRequired(num);
                    Serial.printf("[CMD] Blocked unauthorized max_run edit from client #%u\n", num);
                    break;
                }
                unsigned long mins = doc["value"] | (DEFAULT_MAX_CONT_RUN_MS / 60000UL);
                unsigned long candidate = mins * 60UL * 1000UL;
                candidate = clampULong(candidate, MIN_MAX_CONT_RUN_MS, MAX_MAX_CONT_RUN_MS);
                maxContRunMs = candidate;
                Serial.print("[CMD] Max continuous run updated to: ");
                Serial.print(maxContRunMs / 60000UL);
                Serial.println(" min");
                savePersistentSettings();
                broadcastStatus();
            }

            // --- Handle: Set Cooldown Duration (minutes) ---
            else if (strcmp(action, "cooldown") == 0) {
                if (!isClientAuthed(num)) {
                    sendAuthRequired(num);
                    Serial.printf("[CMD] Blocked unauthorized cooldown edit from client #%u\n", num);
                    break;
                }
                unsigned long mins = doc["value"] | (DEFAULT_COOLDOWN_MS / 60000UL);
                unsigned long candidate = mins * 60UL * 1000UL;
                candidate = clampULong(candidate, MIN_COOLDOWN_MS, MAX_COOLDOWN_MS);
                cooldownMs = candidate;
                Serial.print("[CMD] Cooldown duration updated to: ");
                Serial.print(cooldownMs / 60000UL);
                Serial.println(" min");
                savePersistentSettings();
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
    bool autoState = switchAutoInput.stableLow;
    bool onState = switchOnInput.stableLow;

    String lastState = physicalSwitchState;

    if (autoState) {
        physicalSwitchState = "AUTO";
        autoMode = true;
    } else if (onState) {
        physicalSwitchState = "ON";
        autoMode = false;
        if (!fanOn) {
            if (cooldownActive) {
                // Cooldown overrides physical ON to protect the motor
                if (physicalSwitchState != lastState) {
                    Serial.println("[SWITCH] Physical ON ignored — motor in cooldown.");
                }
            } else {
                Serial.println("[SWITCH] Physical switch turned ON → Forcing Fan ON");
                setFan(true);
            }
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

// ============================================
// Persistent Settings (NVS / Preferences)
// ============================================
void loadPersistentSettings() {
    prefs.begin("fanctrl", true);
    threshold = prefs.getFloat("threshold", DEFAULT_THRESHOLD);
    hysteresis = prefs.getFloat("hyst", DEFAULT_HYSTERESIS);
    maxContRunMs = prefs.getULong("max_run", DEFAULT_MAX_CONT_RUN_MS);
    cooldownMs = prefs.getULong("cooldown", DEFAULT_COOLDOWN_MS);
    totalRunMs = prefs.getULong64("total_ms", 0);
    fanCycleCount = prefs.getULong("cycles", 0);
    prefs.end();

    threshold = clampFloat(threshold, MIN_THRESHOLD, MAX_THRESHOLD);
    hysteresis = clampFloat(hysteresis, MIN_HYSTERESIS, MAX_HYSTERESIS);
    maxContRunMs = clampULong(maxContRunMs, MIN_MAX_CONT_RUN_MS, MAX_MAX_CONT_RUN_MS);
    cooldownMs = clampULong(cooldownMs, MIN_COOLDOWN_MS, MAX_COOLDOWN_MS);

    Serial.println("[NVS] Runtime settings loaded.");
}

void savePersistentSettings() {
    prefs.begin("fanctrl", false);
    prefs.putFloat("threshold", threshold);
    prefs.putFloat("hyst", hysteresis);
    prefs.putULong("max_run", maxContRunMs);
    prefs.putULong("cooldown", cooldownMs);
    prefs.end();
    Serial.println("[NVS] Runtime settings saved.");
}

void saveRuntimeStats() {
    prefs.begin("fanctrl", false);
    prefs.putULong64("total_ms", totalRunMs);
    prefs.putULong("cycles", fanCycleCount);
    prefs.end();
    Serial.println("[NVS] Runtime counters saved.");
}

// ============================================
// WiFi Credentials, Provisioning, mDNS
// ============================================
void loadWiFiCredentials() {
    prefs.begin("wifi", true);
    wifiSsid = prefs.getString("ssid", "");
    wifiPassword = prefs.getString("pass", "");
    prefs.end();

    if (wifiSsid.length() == 0 && String(WIFI_SSID) != "YOUR_WIFI_SSID") {
        wifiSsid = WIFI_SSID;
        wifiPassword = WIFI_PASSWORD;
    }
}

bool connectWiFi() {
    if (wifiSsid.length() == 0) {
        Serial.println("[WIFI] No credentials available.");
        return false;
    }

    Serial.print("[WIFI] Connecting to ");
    Serial.print(wifiSsid);
    Serial.print("...");

    WiFi.mode(WIFI_STA);
    WiFi.setHostname(WIFI_HOSTNAME);
    WiFi.begin(wifiSsid.c_str(), wifiPassword.c_str());

    unsigned long wifiStart = millis();
    bool ledState = false;
    while (WiFi.status() != WL_CONNECTED && (millis() - wifiStart) <= WIFI_TIMEOUT) {
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState ? HIGH : LOW);
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println();
        Serial.println("[WIFI] Connection timeout.");
        return false;
    }

    provisioningMode = false;
    Serial.println();
    Serial.println("[WIFI] Connected!");
    Serial.print("[WIFI] IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("[WIFI] Signal Strength (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    emitEvent("wifi_connected");
    return true;
}

void startProvisioningMode() {
    provisioningMode = true;
    WiFi.mode(WIFI_AP);
    WiFi.softAP(PROVISIONING_AP_SSID, PROVISIONING_AP_PASSWORD);
    IPAddress apIP = WiFi.softAPIP();
    dnsServer.start(53, "*", apIP);

    Serial.println("[WIFI] Provisioning mode started.");
    Serial.print("[WIFI] Join AP: ");
    Serial.println(PROVISIONING_AP_SSID);
    Serial.print("[WIFI] Captive portal: http://");
    Serial.println(apIP);
    emitEvent("provisioning_started");
}

void handleProvisioningRoot() {
    const char page[] PROGMEM = R"html(
<!doctype html><html><head><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Smart Fan WiFi Setup</title>
<style>body{font-family:system-ui,-apple-system,sans-serif;background:#0b0f1e;color:#fff;margin:0;padding:32px}main{max-width:420px;margin:auto;background:rgba(255,255,255,.06);border:1px solid rgba(255,255,255,.12);border-radius:18px;padding:24px}label{display:block;margin:14px 0 6px;color:rgba(255,255,255,.75)}input{box-sizing:border-box;width:100%;padding:12px;border-radius:10px;border:1px solid rgba(255,255,255,.18);background:#131832;color:#fff}button{margin-top:18px;width:100%;padding:12px;border:0;border-radius:10px;background:#00d4ff;color:#08111f;font-weight:700}</style>
</head><body><main><h1>Smart Fan WiFi Setup</h1><p>Enter your WiFi credentials. The controller will save them and restart.</p>
<form method="post" action="/save"><label>WiFi SSID</label><input name="ssid" required maxlength="64"><label>Password</label><input name="pass" type="password" maxlength="64"><button type="submit">Save and Restart</button></form>
</main></body></html>
)html";
    server.send_P(200, "text/html", page);
}

void handleProvisioningSave() {
    String ssid = server.arg("ssid");
    String pass = server.arg("pass");
    ssid.trim();

    if (ssid.length() == 0) {
        server.send(400, "text/plain", "SSID is required.");
        return;
    }

    prefs.begin("wifi", false);
    prefs.putString("ssid", ssid);
    prefs.putString("pass", pass);
    prefs.end();

    server.send(200, "text/html", "<html><body><h1>Saved</h1><p>Restarting controller...</p></body></html>");
    delay(500);
    ESP.restart();
}

void handleNotFound() {
    if (provisioningMode) {
        server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString(), true);
        server.send(302, "text/plain", "");
    } else {
        server.send(404, "text/plain", "Not found");
    }
}

void maintainWiFi() {
    if (WiFi.status() == WL_CONNECTED) return;
    unsigned long now = millis();
    if (now - lastWiFiReconnectAttempt < WIFI_RECONNECT_INTERVAL) return;

    lastWiFiReconnectAttempt = now;
    Serial.println("[WIFI] Disconnected. Attempting reconnect...");
    WiFi.disconnect();
    WiFi.begin(wifiSsid.c_str(), wifiPassword.c_str());
    emitEvent("wifi_reconnect_attempt");
}

void startMDNS() {
    if (mdnsStarted) return;
    if (MDNS.begin(WIFI_HOSTNAME)) {
        MDNS.addService("http", "tcp", 80);
        mdnsStarted = true;
        Serial.print("[MDNS] Started: http://");
        Serial.print(WIFI_HOSTNAME);
        Serial.println(".local");
    } else {
        Serial.println("[MDNS] Failed to start mDNS responder.");
    }
}

// ============================================
// Sensor Fail-Safe / Health
// ============================================
void updateSensorStatus() {
    if (consecutiveSensorFailures >= SENSOR_MAX_FAILURES) {
        sensorStatus = "failed";
    } else if (lastValidSensorRead == 0) {
        sensorStatus = "unknown";
    } else if ((millis() - lastValidSensorRead) > SENSOR_STALE_MS) {
        sensorStatus = "stale";
    } else {
        sensorStatus = "ok";
    }
}

void handleSensorFailure() {
    consecutiveSensorFailures++;
    updateSensorStatus();

    if (consecutiveSensorFailures == SENSOR_MAX_FAILURES) {
        emitEvent("sensor_failed");
        triggerBuzzerEvent(3, 80, 120);
        broadcastStatus();
    }

    if (autoMode && SENSOR_FAIL_SAFE_ON && consecutiveSensorFailures >= SENSOR_MAX_FAILURES) {
        sensorFailSafeActive = true;
        if (!fanOn && !cooldownActive && physicalSwitchState != "OFF") {
            Serial.println("[SENSOR] Fail-safe active → Fan ON while sensor is failed.");
            setFan(true);
        }
    }
}

// ============================================
// Debounce / Controller State Helpers
// ============================================
bool updateDebouncedInput(DebouncedInput &input, unsigned long debounceMs) {
    bool rawLow = (digitalRead(input.pin) == LOW);
    unsigned long now = millis();

    if (rawLow != input.rawLow) {
        input.rawLow = rawLow;
        input.lastRawChange = now;
    }

    if ((now - input.lastRawChange) >= debounceMs && input.stableLow != input.rawLow) {
        input.stableLow = input.rawLow;
        return true;
    }
    return false;
}

void updateControllerState() {
    unsigned long remainingSec = 0;
    String lockType = getLockType(remainingSec);

    if (contactorFault) {
        controllerState = STATE_FAULT;
        activeLockReason = "contactor_fault";
    } else if (cooldownActive) {
        controllerState = STATE_COOLDOWN;
        activeLockReason = "cooldown";
    } else if (lockType == "run") {
        controllerState = STATE_MIN_RUN_LOCK;
        activeLockReason = "minimum_run_time";
    } else if (lockType == "stop") {
        controllerState = STATE_MIN_STOP_LOCK;
        activeLockReason = "minimum_stop_time";
    } else if (fanOn) {
        controllerState = STATE_RUNNING;
        activeLockReason = "none";
    } else if (autoMode) {
        controllerState = STATE_AUTO_WAIT;
        activeLockReason = sensorFailSafeActive ? "sensor_fail_safe" : "none";
    } else {
        controllerState = STATE_OFF;
        activeLockReason = "none";
    }
}

const char* controllerStateName() {
    switch (controllerState) {
        case STATE_OFF: return "off";
        case STATE_AUTO_WAIT: return "auto_wait";
        case STATE_RUNNING: return "running";
        case STATE_MIN_RUN_LOCK: return "min_run_lock";
        case STATE_MIN_STOP_LOCK: return "min_stop_lock";
        case STATE_COOLDOWN: return "cooldown";
        case STATE_FAULT: return "fault";
        default: return "unknown";
    }
}

String wifiQuality() {
    if (provisioningMode) return "provisioning";
    if (WiFi.status() != WL_CONNECTED) return "offline";
    int rssi = WiFi.RSSI();
    if (rssi >= -55) return "excellent";
    if (rssi >= -67) return "good";
    if (rssi >= -75) return "fair";
    return "weak";
}

// ============================================
// WebSocket Helpers / Validation
// ============================================
bool isClientAuthed(uint8_t num) {
    return (num < 10 && clientAuthenticated[num]);
}

void sendAuthRequired(uint8_t num) {
    StaticJsonDocument<128> resDoc;
    resDoc["action"] = "auth_req";
    String responseJson;
    serializeJson(resDoc, responseJson);
    webSocket.sendTXT(num, responseJson);
}

void sendCommandError(uint8_t num, const char *message) {
    StaticJsonDocument<160> resDoc;
    resDoc["action"] = "error";
    resDoc["message"] = message;
    String responseJson;
    serializeJson(resDoc, responseJson);
    webSocket.sendTXT(num, responseJson);
}

float clampFloat(float value, float minValue, float maxValue) {
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

unsigned long clampULong(unsigned long value, unsigned long minValue, unsigned long maxValue) {
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

// ============================================
// Notifications / Buzzer Patterns
// ============================================
void triggerBuzzerEvent(unsigned int beeps, unsigned int onMs, unsigned int offMs) {
    buzzerEventActive = true;
    buzzerEventRemainingToggles = beeps * 2;
    buzzerEventOnMs = onMs;
    buzzerEventOffMs = offMs;
    buzzerEventNextToggle = 0;
    buzzerEventOn = false;
}

void updateBuzzer() {
    unsigned long now = millis();

    if (buzzerEventActive) {
        if (buzzerEventNextToggle == 0 || now >= buzzerEventNextToggle) {
            buzzerEventOn = !buzzerEventOn;
            digitalWrite(BUZZER_PIN, buzzerEventOn ? HIGH : LOW);
            buzzerEventRemainingToggles--;
            buzzerEventNextToggle = now + (buzzerEventOn ? buzzerEventOnMs : buzzerEventOffMs);
            if (buzzerEventRemainingToggles == 0) {
                buzzerEventActive = false;
                buzzerEventOn = false;
                digitalWrite(BUZZER_PIN, LOW);
            }
        }
        return;
    }

    // Continuous alarm pattern. High-temp only sounds if fan is OFF; faults
    // always sound. Sensor fail-safe gets a softer periodic pulse.
    if (contactorFault || (tempAlarm && !fanOn)) {
        digitalWrite(BUZZER_PIN, ((now % 1000) < 200) ? HIGH : LOW);
    } else if (sensorFailSafeActive) {
        digitalWrite(BUZZER_PIN, ((now % 3000) < 120) ? HIGH : LOW);
    } else {
        digitalWrite(BUZZER_PIN, LOW);
    }
}

void emitEvent(const char *eventName) {
    lastNotificationEvent = eventName;
    Serial.print("[EVENT] ");
    Serial.println(eventName);
}
