# 🌡️ ESP32 Smart Fan Controller

A complete IoT system that reads room temperature and automatically controls an induction motor (ceiling fan) ON/OFF using an ESP32 microcontroller. Real-time monitoring and control is available through a beautiful web dashboard served over WiFi.

---

## ✨ Features

- **Automatic fan control** — Fan turns ON/OFF based on configurable temperature threshold
- **Physical selector control** — AUTO/OFF/ON selector switch owns fan mode and power for safer local operation
- **Real-time monitoring** — Live temperature, humidity, and fan status updates via WebSocket
- **Hysteresis control** — Prevents rapid toggling near the threshold temperature
- **Live fan runtime tracking** — Current continuous run, today's runtime, and ON/OFF cycle count update live on the dashboard
- **Forced rest cycle (motor protection)** — After 12 hours of continuous operation the fan is automatically stopped and locked in a 30-minute cooldown to protect the motor windings. Both durations are adjustable from the dashboard.
- **Persistent protected settings** — Threshold, hysteresis, max continuous run, cooldown duration, and runtime counters survive reboot using ESP32 NVS/Preferences
- **Sensor and controller health** — Dashboard reports sensor status, last sensor update, WiFi quality, contactor feedback, controller state, and active lock reason
- **Local 20x4 LCD status** — Optional I2C LCD shows temperature, humidity, fan state, selector position, controller state, WiFi/IP, and cooldown status without opening the dashboard
- **WiFi provisioning mode** — If credentials are missing or invalid, the controller opens a setup access point so WiFi can be configured without editing `config.h`
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
| 9 | 3-position selector switch | AUTO/OFF/ON local control |
| 10 | Contactor auxiliary NO contact | Feedback input for contactor state |
| 11 | Buzzer | Local alarm/notification output |
| 12 | 20x4 I2C LCD Display | HD44780-compatible LCD with I2C backpack, usually address `0x27` or `0x3F` |

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
    │  GPIO 25    ├───────────────────►│ AUTO Switch  │
    │  GPIO 26    ├───────────────────►│ ON Switch    │
    │  GPIO 27    ├───────────────────►│ Aux Feedback │
    │  GPIO 33    ├───────────────────►│ Buzzer       │
    │  GPIO 21    ├───────────────────►│ LCD SDA      │
    │  GPIO 22    ├───────────────────►│ LCD SCL      │
    │  3.3V       ├───────────────────►│ DHT22 VCC    │
    │  5V (Vin)   ├───────────────────►│ LCD VCC      │
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

    20x4 I2C LCD Wiring:
    VCC → ESP32 5V (Vin)
    GND → ESP32 GND
    SDA → ESP32 GPIO 21
    SCL → ESP32 GPIO 22
```

> ⚠️ **SAFETY WARNING**: This system interfaces with 220V AC mains. All AC wiring
> must be performed by a qualified electrician. Use proper insulation and fuses.
> Never work on the circuit while energized.

### Electrical Safety Guidance

This controller should be treated as a mains-voltage appliance, not just a low-voltage ESP32 project. Follow local electrical codes and have the final installation inspected by a qualified electrician.

- **Fuse/MCB protection**: Protect the fan circuit with a correctly rated fuse or miniature circuit breaker sized for the motor nameplate current, startup/inrush current, cable gauge, and local code. Do not rely on the ESP32 relay module as the only protection device.
- **Contactor rating**: Use a contactor rated for the fan motor voltage, full-load current, and inductive motor duty. Choose a device with adequate margin for startup current, not only the steady running current.
- **Snubber/surge suppression**: Add an RC snubber, MOV, or manufacturer-recommended surge suppressor across the contactor coil or load where appropriate. Inductive loads and coils can create voltage spikes that shorten relay life or reset the ESP32.
- **Enclosure**: Mount all mains terminals, relay wiring, contactor terminals, and the power supply inside a non-conductive or properly earthed electrical enclosure with a suitable IP rating for the installation area.
- **Earthing/grounding**: Earth all exposed metal parts, the fan body, and any metal enclosure according to local electrical standards. Keep protective earth continuous and separate from low-voltage signal ground except where the power supply design explicitly requires otherwise.
- **Isolation spacing**: Maintain clear physical separation between 220V AC wiring and ESP32/sensor wiring. Use terminal blocks, insulated crimp ferrules, heat-shrink, and proper creepage/clearance distances; never leave bare mains conductors exposed.
- **Strain relief**: Use cable glands, clamps, or conduit so cable movement cannot pull on screw terminals, solder joints, the relay module, or the ESP32 board.
- **Auxiliary feedback contact**: Connect `FEEDBACK_PIN` only to an isolated dry auxiliary contact from the contactor. Never feed AC voltage into an ESP32 GPIO pin.
- **Testing before load**: First test with the motor disconnected, then with a safe test load, and only then connect the actual fan motor. Verify relay logic, selector positions, contactor feedback, fuse/MCB operation, and emergency power-off access.

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
| LiquidCrystal I2C | Frank de Brabander or compatible | 1.1.x+ |

### Arduino IDE Board Setup

1. Go to **File → Preferences**
2. Add to Board Manager URLs:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Go to **Tools → Board → Board Manager**, search "esp32", install
4. Select **Tools → Board → ESP32 Dev Module**

### Configuration

You can either edit `config.h` before upload or use provisioning mode after flashing.

```cpp
#define WIFI_SSID       "YourWiFiName"      // ← Your home WiFi SSID
#define WIFI_PASSWORD   "YourWiFiPassword"  // ← Your home WiFi password
```

If the firmware still contains the default placeholder SSID, or if the saved credentials cannot connect, the ESP32 starts a setup access point:

| Field | Default |
|-------|---------|
| Setup AP SSID | `SmartFan-Setup` |
| Setup AP password | `fancontroller` |
| Setup page | Open the captive portal or browse to `http://192.168.4.1` |

Optionally adjust:
- `RELAY_PIN` — GPIO for relay (default: 16)
- `DHT_PIN` — GPIO for DHT22 (default: 4)
- `DEFAULT_THRESHOLD` — Auto-ON temperature (default: 28.0°C)
- `DEFAULT_HYSTERESIS` — Hysteresis band (default: 2.0°C)
- `WIFI_HOSTNAME` — mDNS hostname, e.g. `http://smart-fan.local`
- `NTP_SERVER` / `TZ_INFO` — Time source and local timezone used to reset the daily runtime counter at the start of each local day. Default timezone is Bangladesh (`BDT-6`).
- `SETTINGS_PASSWORD` — Administrator password for protected dashboard settings. Change the default before real deployment.
- `SENSOR_FAIL_SAFE_ON` — Whether AUTO mode should run the fan if the DHT22 repeatedly fails and cooldown/local OFF do not block it.
- `LCD_ENABLED` — Enables the optional 20x4 I2C LCD local status display.
- `LCD_I2C_ADDRESS` — LCD backpack address, usually `0x27` or `0x3F`.
- `LCD_SDA_PIN` / `LCD_SCL_PIN` — I2C pins for the LCD, defaulting to ESP32 GPIO 21/22.

### Upload

1. Connect ESP32 via USB
2. Open `fan_control.ino` in Arduino IDE
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
   - **AUTO/OFF/ON selector state**
   - **Physical selector state** and read-only power status
   - **Threshold & hysteresis** sliders for auto mode tuning
   - **Controller health** including sensor status, WiFi quality, feedback status, state, lock reason, and last event

### Auto Mode (Default)
The fan automatically turns ON when temperature rises **above the threshold** and turns OFF when it drops **below (threshold - hysteresis)**. This prevents rapid toggling.

Example with threshold = 28°C, hysteresis = 2°C:
- Fan turns ON at 28°C
- Fan turns OFF at 26°C
- Between 26-28°C: fan stays in its current state

### Physical selector control
Fan mode and power are controlled by the physical **AUTO/OFF/ON** selector switch on the controller enclosure:

| Position | Behavior |
|----------|----------|
| AUTO | Firmware uses temperature threshold, hysteresis, minimum run/stop timers, and cooldown protection. |
| OFF | Fan is forced OFF. This local OFF position overrides dashboard convenience behavior. |
| ON | Fan is forced ON unless a protective cooldown or fault lock blocks the request. |

The dashboard is intentionally read-only for mode and power. It shows the selector state and fan status, while protected settings such as threshold, hysteresis, max continuous run, and cooldown duration require the administrator password.

### Sensor fail-safe behavior
If the DHT22 repeatedly fails to return valid readings, the controller marks the sensor as failed, shows the condition in the dashboard health panel, triggers the configured buzzer notification pattern, and applies the configured safe policy. By default, AUTO mode runs the fan during sensor failure unless cooldown protection or the physical OFF position blocks it.

---

## ⏱️ Runtime Tracking & Forced Rest Cycle

The dashboard's **Fan Runtime** section shows live operational stats:

| Metric | Description |
|--------|-------------|
| Current Run | How long the fan has been ON in its current continuous run (resets to 0 whenever the fan turns OFF). |
| Today Runtime | Total fan ON-time for the current local day. The controller uses NTP time when WiFi is available, with a 24-hour-after-boot fallback if time has not synced. |
| On/Off Cycles | Number of OFF→ON transitions since boot. |
| Rest Cooldown | Status of the motor protection cooldown (`Inactive` or remaining time). |

A progress bar beneath the metrics shows the current run as a percentage of the configured **Max Continuous Run**. It turns red/orange at 85 % to give an early warning that the rest cycle is approaching.

### How the 12 h / 30 min rest cycle works

1. While the fan is ON, the firmware tracks elapsed continuous run-time.
2. When the timer reaches **12 hours** (`DEFAULT_MAX_CONT_RUN_MS`), the firmware:
   - Forces the fan OFF
   - Enters a **30-minute cooldown** (`DEFAULT_COOLDOWN_MS`)
   - Emits a short buzzer beep
   - Shows a blue "MOTOR REST CYCLE" banner on the dashboard with a live countdown
3. During cooldown the firmware blocks:
   - Auto-mode turn-on requests (high temperature is logged but not actuated)
   - Dashboard or network ON requests
   - The physical AUTO/ON selector switch attempting to force the fan ON
4. The physical **OFF** position still works at all times — safety overrides convenience.
5. When the 30 min elapses the fan resumes normal AUTO/OFF/ON selector-driven operation. The continuous-run timer resets to zero on every OFF, so a long off-period naturally pre-empts the rest cycle.

### Adjustable from the dashboard

The **Rest Cycle** card on the dashboard exposes two password-protected sliders:

| Setting | Range | Default |
|---------|-------|---------|
| Max Continuous Run | 1 h – 24 h (30 min steps) | 12 h |
| Cooldown Duration | 5 min – 120 min (5 min steps) | 30 min |

Click the 🔒 icon and enter the administrator password (defined by `SETTINGS_PASSWORD` in `config.h`) to unlock the sliders.

Runtime-adjustable settings are saved to ESP32 NVS/Preferences. Threshold, hysteresis, max continuous run, cooldown duration, today's runtime, and cycle count are restored after reboot. Today's runtime resets when the local calendar day changes after NTP sync.

### Emergency override

If a cooldown is active and you absolutely must run the fan immediately, an **End Cooldown** button appears beneath the runtime metrics. It is only available to authenticated users and asks for explicit confirmation, because bypassing the rest cycle repeatedly can shorten the motor's lifespan.

### Notifications
The buzzer uses non-blocking patterns for important local events such as high temperature, contactor fault, repeated sensor failure, and cooldown start/end. The WebSocket status payload also includes a `last_event` field so future network alert integrations can hook into the same event stream.

---

## 🧪 Preview Mode

To preview the web dashboard without hardware, open `index.html` in any browser. It runs with simulated temperature data that fluctuates over time.

The preview also demonstrates the rest cycle on a **compressed timeline**: the fan triggers the cooldown after **12 seconds** of continuous run and rests for **8 seconds** (instead of 12 h / 30 min). The Rest Cycle sliders still display the real-world 12 h / 30 min defaults so you can see the production UI exactly as it will appear on the device.

---

## 📁 Project Structure

```
esp32-fan-control/
├── fan_control.ino        # Main ESP32 Arduino sketch
├── config.h               # WiFi, pin, and threshold configuration
├── web_page.h             # Dashboard HTML (PROGMEM, auto-generated)
├── index.html             # Standalone dashboard preview/source HTML
├── generate_web_page.py   # Regenerates web_page.h from index.html
├── sync_web_page.ps1      # PowerShell helper for syncing the web dashboard
└── README.md              # This file
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
2. **ESP32** applies threshold logic in AUTO or follows the physical OFF/ON selector positions.
3. **Relay Module** switches control voltage to energize or de-energize the **Magnetic Contactor's coil**.
4. **Magnetic Contactor** uses its heavy-duty power contacts to switch high-inrush AC mains to the **Exhaust Fan Motor** ON or OFF. This isolates the sensitive relay module from the motor's inductive startup surges, protecting contacts and ensuring longevity.
5. **Web dashboard** shows real-time status via WebSocket and allows protected settings changes after administrator unlock.

---

## 📄 License

This project is open source. Use it freely for educational and personal purposes.
