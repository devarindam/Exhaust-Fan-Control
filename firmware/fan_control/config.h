#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// WiFi Configuration (Station Mode)
// ============================================
#define WIFI_SSID       "YOUR_WIFI_SSID"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"

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

// Cycle Protection Timers (milliseconds)
#define MIN_RUN_TIME_MS     30000   // Fan must run at least 30s once turned ON
#define MIN_STOP_TIME_MS    30000   // Fan must stay OFF at least 30s once turned OFF

// ============================================
// Timing (milliseconds)
// ============================================
#define SENSOR_INTERVAL     2000    // Read sensor every 2 seconds
#define BROADCAST_INTERVAL  1000    // Broadcast status every 1 second
#define WIFI_TIMEOUT        20000   // WiFi connection timeout

#endif
