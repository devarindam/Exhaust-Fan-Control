# 🌡️ ESP32 Smart Fan Controller

A complete IoT system that reads room temperature and automatically controls an induction motor (ceiling fan) ON/OFF using an ESP32 microcontroller. Real-time monitoring and control is available through a beautiful web dashboard served over WiFi.

---

## ✨ Features

- **Automatic fan control** — Fan turns ON/OFF based on configurable temperature threshold
- **Physical switch control** — Two toggle switches (AUTO/MANUAL + ON/OFF) own fan mode and power for safer local operation, with the AUTO toggle taking priority
- **Real-time monitoring** — Live temperature, humidity, and fan status updates via WebSocket
- **Hysteresis control** — Prevents rapid toggling near the threshold temperature
- **Live fan runtime tracking** — Current continuous run, today's runtime, and ON/OFF cycle count update live on the dashboard
- **Forced rest cycle (motor protection)** — After 12 hours of continuous operation the fan is automatically stopped and locked in a 30-minute cooldown to protect the motor windings. Both durations are adjustable from the dashboard.
- **Persistent protected settings** — Threshold, hysteresis, max continuous run, cooldown duration, and runtime counters survive reboot using ESP32 NVS/Preferences
- **Sensor and controller health** — Dashboard reports sensor status, last sensor update, WiFi quality, contactor feedback, controller state, and active lock reason
- **Local 20x4 LCD status** — Optional I2C LCD shows temperature, humidity, fan state, switch state, controller state, WiFi/IP, and cooldown status without opening the dashboard
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
| 9 | 2 × SPST toggle switches | AUTO/MANUAL toggle (GPIO 25) + ON/OFF toggle (GPIO 26), active-LOW |
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
    │  GPIO 25    ├───────────────────►│ Auto/Manual  │
    │  GPIO 26    ├───────────────────►│ On/Off Switch│
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

    Control Switches & Auxiliary Feedback (all Active LOW):

    3.3V ─────────────┐  (internal pull-ups enabled in firmware)
                      │
    AUTO/MANUAL SW    │            ON/OFF SW
    ┌──────────┐      │            ┌──────────┐
    │  SPST    │      │            │  SPST    │
    │ toggle   │      │            │ toggle   │
    └─┬──────┬─┘      │            └─┬──────┬─┘
      │      │        │              │      │
   GPIO 25   └─ GND   │           GPIO 26   └─ GND
      │               │              │
      └─ (INPUT_PULLUP)              └─ (INPUT_PULLUP)

      Closed (ON)  → pin reads LOW  → active
      Open  (OFF)  → pin reads HIGH → inactive


    Contactor Auxiliary Feedback (dry NO contact):

    ┌─────────────────────────────┐
    │      MAGNETIC CONTACTOR     │
    │   ┌───────────────────────┐ │
    │   │  Auxiliary NO contact │ │
    │   └──────┬─────────┬──────┘ │
    └──────────┼─────────┼────────┘
               │         │
            GPIO 27      └──── ESP32 GND
               │
        (INPUT_PULLUP, Active LOW)

      Contactor energized → aux NO closes → GPIO 27 LOW  → "fan confirmed ON"
      Contactor released  → aux NO opens  → GPIO 27 HIGH → "fan confirmed OFF"

    ⚠️ The auxiliary contact must be a DRY (volt-free) contact carrying only
       ESP32 logic level. NEVER connect 220V AC to GPIO 27.

```

### DHT22 Data Line Pull-Up

The DHT22 communicates with the ESP32 over a single bidirectional data line. This line must be held HIGH while idle so the sensor and microcontroller can reliably drive it LOW during each transaction. Without a pull-up the line floats, producing intermittent reads, checksum failures, or a permanently "failed" sensor.

- **Resistor**: Connect a 10kΩ resistor (4.7kΩ–10kΩ acceptable) between the DHT22 **Data** pin (Pin 2) and **3.3V**. The ESP32's `DHT_PIN` (default GPIO 4) ties into the same node.
- **Wire to 3.3V, not 5V**: The ESP32 GPIO pins are not 5V-tolerant. Pull the data line up to 3.3V — the same rail powering the DHT22 VCC — so the idle-HIGH level never exceeds the ESP32's input rating.
- **Keep leads short**: For runs longer than ~20 cm, use the lower 4.7kΩ value and/or shielded cable to keep edges clean.

```
    3.3V ──┬──────────────► DHT22 Pin 1 (VCC)
           │
          [ ] 10kΩ
           │
           ├──────────────► DHT22 Pin 2 (DATA)
           │
           └──────────────► ESP32 GPIO 4
    GND ──────────────────► DHT22 Pin 3 (GND)
```

> **Note on breakout boards**: Many DHT22 modules sold on a small PCB (typically 3-pin breakouts) already include an on-board pull-up resistor. If you are using such a module, do **not** add a second external resistor — the parallel resistance can pull the line too strongly. A bare 4-pin DHT22 sensor always requires the external pull-up shown above. If unsure, check for a resistor next to the data pin or measure the resistance between the DATA and VCC pins.

### Opto-Isolated Relay Module (1-Channel, 5V)

This project uses a 5V opto-isolated relay module with pins `DC+`, `DC-`, `IN` (control side), `JD+`, `JD-` (relay-coil power), and `COM` / `NO` / `NC` (output screw terminals). The relay output switches the **contactor coil**, not the motor directly. The board has two jumpers that must be set correctly.

**Trigger jumper → set to L (low-level).** The firmware uses active-LOW relay logic (`RELAY_ON = LOW` in `config.h`): when it wants the fan ON it drives `RELAY_PIN` (GPIO 16) LOW. The module must therefore interpret LOW as "energize", which is the **L** jumper position. Leaving it on **H** inverts the logic — the fan would run when the firmware thinks it is off, and vice-versa.

**JD+/JD- jumper → choose your coil-power source.** The relay coil draws ~190 mA. The board ships with a shorting jumper tying `DC+` to `JD+`, so the coil runs from the same 5V as the control logic. You have two options:

- **Option A — shared 5V (jumper installed, simplest)**: `DC+`/`DC-`/`JD+`/`JD-` all run from one 5V supply. Size that supply for at least ~1 A to cover the ESP32, relay coil, LCD, and buzzer together. Coil switching noise shares the ESP32 rail.
- **Option B — separate coil supply (jumper removed, full isolation, recommended for long-term use)**: Remove the `DC+`/`JD+` jumper and feed `JD+`/`JD-` from an independent 5V source (e.g. a second HLK-PM01 or 5V adapter). **Keep `JD-` on that separate supply's ground — do NOT join it to the ESP32 ground**, or the opto isolation is defeated. The control side (`DC+`/`DC-`) then only powers the tiny opto LED (~2–4 mA).

```
        ESP32                  5V Opto-Isolated Relay Module
   ┌─────────────┐            ┌────────────────────────────────┐
   │             │            │  CONTROL SIDE  │  COIL SIDE    │
   │  GPIO 16  ──┼───────────►│ IN             │               │
   │  5V       ──┼───────────►│ DC+      ┌─────┤ JD+           │
   │  GND      ──┼───────────►│ DC-      │     │ JD-           │
   │             │            │  [H][L]  │     │               │
   │             │            │     ▲    └─────┘ DC+–JD+ jumper│
   └─────────────┘            │  set L          (Option A) or  │
                              │ (active-LOW)    separate 5V to │
                              │                 JD+/JD-(Opt B) │
                              ├────────────────────────────────┤
                              │  OUTPUT (screw terminals)      │
                              │   COM ──► 220V AC LIVE         │
                              │   NO  ──► Contactor Coil A1    │
                              │   NC  ──► (unused)             │
                              └────────────────────────────────┘
                                          │
                                Contactor Coil A2 ──► 220V NEUTRAL

      Using NO (not NC) means the contactor — and fan — is OFF when the
      relay is de-energized or the ESP32 is unpowered (fail-safe state).
```

> **Inductive load**: Fit an RC snubber or MOV across the contactor coil (see the safety section). The relay contacts see the coil's inductive kickback on every switch, and the ~30 A contact rating should be treated conservatively — this is exactly why the relay switches a contactor rather than the motor's inductive inrush directly.

### Contactor Coil Snubber (RC + MOV)

To protect the relay contacts from the contactor coil's inductive kickback (and reduce electrical noise that can disturb the ESP32), fit a snubber across the coil. The simplest option is a ready-made combined **RC + MOV absorption module** (sold for relay/thyristor contact protection, typically rated for AC/DC 5–400 V inductive loads under 1000 W). The RC network damps the voltage ringing while the built-in varistor clamps the peak, so one part covers both jobs.

- **Placement**: connect the module's two leads directly across the contactor **coil terminals (A1–A2)**. (Connecting across the relay's COM–NO contacts is also valid.) It is AC-rated and non-polarized, so lead orientation does not matter.
- **If building discrete instead of a module**: a 0.1 µF **X2 safety capacitor (275 VAC)** in series with a **100 Ω, 2 W** resistor across A1–A2 is a suitable equivalent for a small AC coil; add a ~275 VAC-class MOV (e.g. S14K275) in parallel for peak clamping.
- **Sizing note**: the RC values scale with the coil's holding current/VA — for a typical low-VA 220 V AC contactor coil the values above (or a generic module) are appropriate. A much larger coil would need a larger capacitor.
- **DC coils differ**: if you ever use a **DC** contactor coil instead, do not use an RC snubber — fit a simple flyback diode across the coil.

```
   Relay NO ──────► Contactor Coil A1 ──┐
                                        ├──[ RC + MOV snubber module ]
   220V Neutral ──► Contactor Coil A2 ──┘   (across A1–A2)
```

> Mount the snubber at the contactor coil terminals with short leads, inside the enclosure. It sits on 220 V AC — treat it as a mains-voltage part and verify the encapsulation is intact before wiring.

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
- **Testing before load**: First test with the motor disconnected, then with a safe test load, and only then connect the actual fan motor. Verify relay logic, switch states, contactor feedback, fuse/MCB operation, and emergency power-off access.

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
   - **Resolved switch state** (AUTO / ON / OFF)
   - **Physical switch state** and read-only power status
   - **Threshold & hysteresis** sliders for auto mode tuning
   - **Controller health** including sensor status, WiFi quality, feedback status, state, lock reason, and last event

### Auto Mode (Default)
The fan automatically turns ON when temperature rises **above the threshold** and turns OFF when it drops **below (threshold - hysteresis)**. This prevents rapid toggling.

Example with threshold = 28°C, hysteresis = 2°C:
- Fan turns ON at 28°C
- Fan turns OFF at 26°C
- Between 26-28°C: fan stays in its current state

### Physical switch control
Fan mode and power are controlled by **two SPST toggle switches** on the controller enclosure — an **AUTO/MANUAL** toggle (GPIO 25) and an **ON/OFF** toggle (GPIO 26). Both are active-LOW (closed = active). The firmware resolves them into a single state, with the AUTO toggle taking priority:

| AUTO/MANUAL toggle | ON/OFF toggle | Resolved state | Behavior |
|--------------------|---------------|----------------|----------|
| Active | (ignored) | AUTO | Firmware uses temperature threshold, hysteresis, minimum run/stop timers, and cooldown protection. |
| Inactive | Active | ON | Fan is forced ON unless a protective cooldown or fault lock blocks the request. |
| Inactive | Inactive | OFF | Fan is forced OFF. This local OFF state overrides dashboard convenience behavior. |

Because the AUTO toggle is checked first, enabling it overrides the ON/OFF toggle regardless of the latter's position. The "OFF" state is simply both toggles inactive.

The dashboard is intentionally read-only for mode and power. It shows the resolved switch state and fan status, while protected settings such as threshold, hysteresis, max continuous run, and cooldown duration require the administrator password.

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
   - The physical AUTO or ON toggle attempting to force the fan ON
4. The physical **OFF** position still works at all times — safety overrides convenience.
5. When the 30 min elapses the fan resumes normal switch-driven operation (AUTO / ON / OFF). The continuous-run timer resets to zero on every OFF, so a long off-period naturally pre-empts the rest cycle.

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
Exhaust-Fan-Control-main/
├── README.md                        # This file
├── generate_web_page.py             # Regenerates web_page.h from index.html
├── sync_web_page.ps1                # PowerShell helper for syncing the web dashboard
├── preview/
│   └── index.html                   # Standalone dashboard preview/source HTML
└── firmware/
    ├── fan_control.ino          # Main ESP32 Arduino sketch
    ├── config.h                 # WiFi, pin, and threshold configuration
    └── web_page.h               # Dashboard HTML (PROGMEM, auto-generated)
```

---

## 🔌 How It Works

```
┌──────────┐     GPIO 4      ┌────────┐
│  DHT22   ├─────────────────┤        │      WiFi (STA)       ┌──────────┐
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
2. **ESP32** applies threshold logic in AUTO or follows the physical ON/OFF toggle state.
3. **Relay Module** switches control voltage to energize or de-energize the **Magnetic Contactor's coil**.
4. **Magnetic Contactor** uses its heavy-duty power contacts to switch high-inrush AC mains to the **Exhaust Fan Motor** ON or OFF. This isolates the sensitive relay module from the motor's inductive startup surges, protecting contacts and ensuring longevity.
5. **Web dashboard** shows real-time status via WebSocket and allows protected settings changes after administrator unlock.

---

## 📄 License

This project is open source. Use it freely for educational and personal purposes.
