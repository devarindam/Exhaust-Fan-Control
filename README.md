# 🌡️ ESP32 Smart Fan Controller

A complete IoT system that reads room temperature and automatically controls an induction motor (ceiling fan) ON/OFF using an ESP32 microcontroller. Real-time monitoring and control is available through a beautiful web dashboard served over WiFi.

---

## ✨ Features

- **Automatic fan control** — Fan turns ON/OFF based on configurable temperature threshold
- **Manual override** — Toggle fan manually via the web dashboard
- **Real-time monitoring** — Live temperature, humidity, and fan status updates via WebSocket
- **Hysteresis control** — Prevents rapid toggling near the threshold temperature
- **Beautiful dashboard** — Premium dark-themed web UI with animated fan icon
- **Zero app install** — Access from any device's browser on the same WiFi network

---

## 🔧 Hardware Required

| # | Component | Specification |
|---|-----------|---------------|
| 1 | ESP32 DevKit V1 | 38-pin, WiFi + BLE |
| 2 | DHT22 Sensor | Temperature & humidity (-40°C to 80°C, ±0.5°C) |
| 3 | Relay Module | 5V opto-isolated, single-channel, 10A @ 250VAC |
| 4 | Magnetic Contactor| 220V AC Coil, 3-pole or 2-pole contacts (rated for fan motor load) |
| 5 | Induction Motor | Single-phase exhaust fan (220V AC) |
| 6 | 10kΩ Resistor | Pull-up for DHT22 data line |
| 7 | 5V Power Supply | For ESP32 (USB adapter or HLK-PM01) |
| 8 | Jumper Wires | For connections |

---

## 📌 Wiring Diagram

```
                 ┌────────────────────────────────────────────────────────┐
                 │                   AC MAINS (220V)                      │
                 │              LIVE ───────────┬──────── NEUTRAL         │
                 │                              │            │            │
                 │                       ┌──────┴──────┐     │            │
                 │                       │ Optoisolated│     │            │
                 │                       │    Relay    │     │            │
                 │                       │  (GPIO16)   │     │            │
                 │                       └──────┬──────┘     │            │
                 │                              │            │            │
                 │                       ┌──────▼──────┐     │            │
                 │                       │ContactorCoil│     │            │
                 │                       │  (A1 / A2)  ├─────┤            │
                 │                       └─────────────┘     │            │
                 │                                           │            │
                 │                       ┌─────────────┐     │            │
                 │            LIVE ──────┤  Contactor  │     │            │
                 │                       │Main Contacts│     │            │
                 │                       │  (L1 / T1)  │     │            │
                 │                       └──────┬──────┘     │            │
                 │                              │            │            │
                 │                       ┌──────▼──────┐     │            │
                 │                       │ EXHAUST FAN │     │            │
                 │                       │   MOTOR     ├─────┘            │
                 │                       └─────────────┘                  │
                 └────────────────────────────────────────────────────────┘

    ESP32 DevKit                           Components
    ┌─────────────┐                    ┌──────────────┐
    │  GPIO 4     ├───────────────────►│ DHT22 Data   │
    │  GPIO 16    ├───────────────────►│ Relay IN     │
    │  3.3V       ├───────────────────►│ DHT22 VCC    │
    │  5V (Vin)   ├───────────────────►│ Relay VCC    │
    │  GND        ├───────────────────►│ Common GND   │
    │  GPIO 2     │ (Onboard LED)      │              │
    └─────────────┘                    └──────────────┘

    DHT22 Wiring:
    ┌─────────┐
    │  DHT22  │
    │ 1  2  3 │    Pin 1 (VCC)  → ESP32 3.3V
    └─┬──┬──┬─┘    Pin 2 (Data) → ESP32 GPIO 4 (+ 10kΩ pull-up to 3.3V)
      │  │  │      Pin 3 (GND)  → ESP32 GND
     VCC DATA GND
```

> ⚠️ **SAFETY WARNING**: This system interfaces with 220V AC mains. All AC wiring
> must be performed by a qualified electrician. Use proper insulation and fuses.
> Never work on the circuit while energized.

---

## 💻 Software Setup

### Prerequisites

Install these libraries in Arduino IDE (**Sketch → Include Library → Manage Libraries**):

| Library | Author | Version |
|---------|--------|---------|
| DHT sensor library | Adafruit | 1.4.x+ |
| Adafruit Unified Sensor | Adafruit | 1.1.x+ |
| ArduinoJson | Benoît Blanchon | 6.x |
| WebSockets | Markus Sattler (Links2004) | 2.4.x+ |

### Arduino IDE Board Setup

1. Go to **File → Preferences**
2. Add to Board Manager URLs:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Go to **Tools → Board → Board Manager**, search "esp32", install
4. Select **Tools → Board → ESP32 Dev Module**

### Configuration

Edit `firmware/fan_control/config.h`:

```cpp
#define WIFI_SSID       "YourWiFiName"      // ← Your home WiFi SSID
#define WIFI_PASSWORD   "YourWiFiPassword"  // ← Your home WiFi password
```

Optionally adjust:
- `RELAY_PIN` — GPIO for relay (default: 16)
- `DHT_PIN` — GPIO for DHT22 (default: 4)
- `DEFAULT_THRESHOLD` — Auto-ON temperature (default: 28.0°C)
- `DEFAULT_HYSTERESIS` — Hysteresis band (default: 2.0°C)

### Upload

1. Connect ESP32 via USB
2. Open `firmware/fan_control/fan_control.ino` in Arduino IDE
3. Select the correct COM port under **Tools → Port**
4. Click **Upload** (→)
5. Open **Serial Monitor** (115200 baud) to see the assigned IP address

---

## 📱 Using the Dashboard

1. Ensure your phone/laptop is on the **same WiFi network** as the ESP32
2. Open a browser and go to the IP address shown in Serial Monitor (e.g., `http://192.168.1.100`)
3. The dashboard shows:
   - **Live temperature** and humidity
   - **Fan status** with animated fan icon
   - **Auto/Manual mode** toggle
   - **Power button** for manual ON/OFF control
   - **Threshold & hysteresis** sliders for auto mode tuning

### Auto Mode (Default)
The fan automatically turns ON when temperature rises **above the threshold** and turns OFF when it drops **below (threshold - hysteresis)**. This prevents rapid toggling.

Example with threshold = 28°C, hysteresis = 2°C:
- Fan turns ON at 28°C
- Fan turns OFF at 26°C
- Between 26-28°C: fan stays in its current state

### Manual Mode
Switch to Manual mode on the dashboard. Use the power button to toggle the fan ON/OFF directly.

---

## 🧪 Preview Mode

To preview the web dashboard without hardware, open `preview/index.html` in any browser. It runs with simulated temperature data that fluctuates over time.

---

## 📁 Project Structure

```
esp32-fan-control/
├── firmware/
│   └── fan_control/
│       ├── fan_control.ino      # Main ESP32 Arduino sketch
│       ├── config.h             # WiFi, pin, and threshold configuration
│       └── web_page.h           # Dashboard HTML (PROGMEM, auto-generated)
├── preview/
│   └── index.html               # Standalone dashboard preview (simulated data)
└── README.md                    # This file
```

---

## 🔌 How It Works

```
┌──────────┐     GPIO 4      ┌────────┐
│  DHT22   ├─────────────────┤        │      WiFi (STA)      ┌──────────┐
│  Sensor  │   Temperature   │ ESP32  ├──────────────────────►│  Browser │
└──────────┘   & Humidity    │        │   WebSocket (:81)     │Dashboard │
                             │        │   HTTP (:80)          └──────────┘
┌──────────┐     GPIO 16     │        │
│  Relay   ├─────────────────┤        │
│  Module  │   ON/OFF Signal │        │
└────┬─────┘                 └────────┘
     │
     │ (Coil Voltage)
┌────▼─────┐
│ Magnetic │
│Contactor │
└────┬─────┘
     │ (220V AC Load Switching)
┌────▼─────┐
│ EXHAUST  │
│   FAN    │  (220V AC Induction Motor)
└──────────┘
```

1. **DHT22** reads room temperature every 2 seconds.
2. **ESP32** applies threshold logic (auto mode) or user commands (manual mode).
3. **Relay Module** switches control voltage to energize or de-energize the **Magnetic Contactor's coil**.
4. **Magnetic Contactor** uses its heavy-duty power contacts to switch high-inrush AC mains to the **Exhaust Fan Motor** ON or OFF. This isolates the sensitive relay module from the motor's inductive startup surges, protecting contacts and ensuring longevity.
5. **Web dashboard** shows real-time status via WebSocket and allows control.

---

## 📄 License

This project is open source. Use it freely for educational and personal purposes.
