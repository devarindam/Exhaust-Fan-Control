#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// WiFi Configuration (Station Mode)
// ============================================
#define WIFI_SSID       "YOUR_WIFI_SSID"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"
#define WIFI_HOSTNAME   "smart-fan"

// Captive portal provisioning fallback. If WiFi credentials are empty/default
// or the controller cannot connect, it starts this AP so credentials can be set
// from a phone/laptop without editing this file.
#define PROVISIONING_AP_SSID       "SmartFan-Setup"
#define PROVISIONING_AP_PASSWORD   "fancontroller"  // 8+ chars required by ESP32 SoftAP

// ============================================
// Pin Configuration
// ============================================
#define DHT_PIN             4       // DHT22 data pin
#define DHT_TYPE            DHT22   // Sensor type
#define RELAY_PIN           16      // Relay control pin
#define LED_PIN             2       // Onboard LED (status)

// New GPIO pins (Safe, non-strapping pins)
#define SWITCH_AUTO_PIN     25      // Physical switch AUTO position (LOW when active)
#define SWITCH_ON_PIN       26      // Physical switch ON position (LOW when active)
#define FEEDBACK_PIN        27      // Contactor aux NO contact (LOW when closed/energized)
#define BUZZER_PIN          33      // Buzzer pin

// ============================================
// Relay Logic
// Most relay modules are active-LOW
// ============================================
#define RELAY_ON            LOW
#define RELAY_OFF           HIGH

// ============================================
// Default Thresholds & Protection Settings
// ============================================
#define DEFAULT_THRESHOLD   28.0    // Fan turns ON above this temp (°C)
#define DEFAULT_HYSTERESIS  2.0     // Fan turns OFF below (threshold - hysteresis)
#define ALARM_THRESHOLD     35.0    // Alarm triggers at/above 35.0°C
#define ALARM_HYSTERESIS    1.0     // Alarm turns off below (threshold - hysteresis)
#define FEEDBACK_DELAY_MS   1000    // Wait 1s for contactor to pull in/release before testing
#define SETTINGS_PASSWORD   "admin123"  // Password to change threshold/hysteresis

// Runtime-adjustable bounds used by WebSocket command validation
#define MIN_THRESHOLD       18.0
#define MAX_THRESHOLD       40.0
#define MIN_HYSTERESIS      0.5
#define MAX_HYSTERESIS      5.0

// Cycle Protection Timers (milliseconds)
#define MIN_RUN_TIME_MS     30000   // Fan must run at least 30s once turned ON
#define MIN_STOP_TIME_MS    30000   // Fan must stay OFF at least 30s once turned OFF

// ============================================
// Continuous-Run Protection (Motor Rest Cycle)
// ============================================
// If the fan runs continuously for MAX_CONT_RUN_MS, it is forced OFF and locked
// in a cooldown state for COOLDOWN_MS. The continuous-run timer resets to zero
// every time the fan turns OFF (for any reason).
//
// These defaults can be changed at runtime from the dashboard (password-
// protected). Bounds are enforced server-side.
#define DEFAULT_MAX_CONT_RUN_MS   (12UL * 60UL * 60UL * 1000UL)   // 12 hours
#define DEFAULT_COOLDOWN_MS       (30UL * 60UL * 1000UL)          // 30 minutes

// Allowed runtime-configurable bounds
#define MIN_MAX_CONT_RUN_MS       (1UL * 60UL * 60UL * 1000UL)    // 1 hour
#define MAX_MAX_CONT_RUN_MS       (24UL * 60UL * 60UL * 1000UL)   // 24 hours
#define MIN_COOLDOWN_MS           (5UL * 60UL * 1000UL)           // 5 minutes
#define MAX_COOLDOWN_MS           (120UL * 60UL * 1000UL)         // 120 minutes

// ============================================
// Sensor Fail-Safe
// ============================================
#define SENSOR_MAX_FAILURES        3       // Mark failed after this many consecutive DHT failures
#define SENSOR_STALE_MS            10000   // Mark stale if no valid reading for 10s
#define SENSOR_FAIL_SAFE_ON        true    // In AUTO, run fan on sensor failure unless cooldown/physical OFF blocks it

// ============================================
// Debounce / Filtering
// ============================================
#define SWITCH_DEBOUNCE_MS         60
#define FEEDBACK_DEBOUNCE_MS       120

// ============================================
// Timing (milliseconds)
// ============================================
#define SENSOR_INTERVAL     2000    // Read sensor every 2 seconds
#define BROADCAST_INTERVAL  1000    // Broadcast status every 1 second
#define WIFI_TIMEOUT        20000   // WiFi connection timeout
#define WIFI_RECONNECT_INTERVAL 10000

#endif
