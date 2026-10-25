# WTEC: Arduino-Based Smart Classroom Occupancy & Status Tracker

> An embedded IoT prototype developed at Hankuk University of Foreign Studies (Biomedical Engineering, 2023).  
> WTEC monitors real-time lecture status via RFID authentication and tracks room occupancy using dual infrared (IR) beam sensors, broadcasting live room availability over Bluetooth.

---

## 📌 Project Overview

* **Problem Statement**: Students often spend significant time wandering campus halls to locate empty classrooms for study or teamwork during breaks.
* **Proposed Solution**: A decentralized embedded monitoring station installed at classroom doors. It tracks physical entry/exit count and lecture status, allowing students to query availability remotely over Bluetooth.
* **Key Capabilities**:
  * **Lecture State Tracking**: RFID card verification to toggle status between "In Class" and "Available".
  * **Directional Movement Counting**: Dual IR obstacle sensors detecting bidirectional transit (Entry / Exit).
  * **Wireless Query Interface**: Interactive SoftwareSerial Bluetooth link reporting occupancy count and remaining seat capacity.

---

## 🛠 Tech Stack & Hardware Components

| Category | Component / Library | Technical Role |
| :--- | :--- | :--- |
| **MCU** | Arduino Uno R3 (ATmega328P) | Central processing for sensor acquisition and state control |
| **Wireless** | HC-05 / HM-10 Bluetooth Module | UART-based terminal interface (9600 bps) |
| **Authentication** | RC522 RFID Reader | SPI-based UID matching for lecture verification |
| **Sensing** | Dual Infrared Obstacle Sensors | Directional beam-break detection via digital I/O |
| **Software** | `<SPI.h>`, `<MFRC522.h>`, `<SoftwareSerial.h>` | Hardware peripheral drivers and serial communication |

---

## 🔌 Hardware Wiring & Pin Mapping

All components share common power rails distributed from an Arduino Uno via breadboard connections.

| Peripheral | Arduino Uno Pin | Protocol / Signal Type | Operating Voltage |
| :--- | :--- | :--- | :--- |
| **RC522 RFID** | Pin 10 (SS), Pin 9 (RST) | SPI (SCK: 13, MISO: 12, MOSI: 11) | 3.3V |
| **Bluetooth Module** | Pin 4 (Arduino TX), Pin 5 (Arduino RX) | Software UART (Cross-connected) | 5.0V |
| **IR Sensor 1 (Exterior)** | Pin 2 | Digital Input (`LOW` when triggered) | 5.0V |
| **IR Sensor 2 (Interior)** | Pin 3 | Digital Input (`LOW` when triggered) | 5.0V |

<p align="center">
  <img src="docs/images/circuit_overview.png" width="48%" alt="Circuit Overview" />
  <img src="docs/images/circuit_power_on.png" width="48%" alt="Circuit Power-On Verification" />
</p>

---

## 📐 System Flow

1. **Authentication (RC522 RFID)**:
   * Reads 4-byte Card UID over SPI.
   * Compares the current UID against the previous recorded state: toggles room state (`Class = 0` / `Class = 1`).
2. **Occupancy Counting (Dual IR Sensors)**:
   * Exterior Sensor 1 triggered first, then Interior Sensor 2: **Entry Event (`count++`)**.
   * Interior Sensor 2 triggered first, then Exterior Sensor 1: **Exit Event (`count--`)**.
   * Remaining seats are derived from nominal room capacity: `remaining = 50 - count`.
3. **Bluetooth Terminal Interaction**:
   * Users enter command options (`1: Search`, `2: Exit`).
   * When queried with a valid classroom number (e.g., `306`), the system responds with status, current count, and available seats.

---

## 🔍 Engineering Challenges & Troubleshooting

### 1. IR Beam Dispersion & False Triggering
* **Issue**: The initial wide-angle PIR/IR sensors caused continuous triggers due to surface reflections and wide ambient coverage.
* **Resolution**: Replaced them with narrow, direct line-of-sight active IR beam sensors with onboard potentiometers to restrict detection strictly across the door threshold.

### 2. Event-Loop Starvation & Blocking Delay Bug
* **Issue**: Integrating separate RFID and IR modules resulted in missed counts. A global `delay()` at the end of the RFID read cycle suspended the main `loop()`, causing the system to miss fast-walking pedestrians.
* **Resolution**: Scoped delay calls strictly inside the UID match execution block, keeping the polling loop responsive for continuous IR sensor sampling.

### 3. Non-Blocking Timestamp Transition (`millis()`)
* **Issue**: Relying on fixed blocking timers (`delay(800)`) between sensors required people to pass at a rigid walking speed to register counts.
* **Resolution**: Designed an asynchronous timestamp state tracker using `millis()` to validate transit intervals dynamically:

```cpp
if (bool1 && bool2) {
  if (abs(time1 - time2) < inOutInterval) {
    if (time1 < time2) {
      count++; // Valid Entry
    } else if (time2 < time1 && count > 0) {
      count--; // Valid Exit
    }
  }
  // Reset trigger flags
  bool1 = false;
  bool2 = false;
}
```

---

## 🏫 Prototype Model Verification

A 1:20 scale model classroom was constructed with foam boards to evaluate mechanical sensor placement, beam-break alignment, and communication reliability during simulated foot traffic.

<p align="center">
  <img src="docs/images/prototype_exterior.png" width="48%" alt="Prototype Exterior & Wall Assembly" />
  <img src="docs/images/prototype_interior.png" width="48%" alt="Prototype Interior & Sensor Placement" />
</p>

---

## 📁 Repository Structure

```text
├── docs/
│   └── images/
│       ├── circuit_overview.png     # Breadboard circuit wiring
│       ├── circuit_power_on.png     # Power-on verification
│       ├── prototype_exterior.png   # Exterior wall & mounting setup
│       └── prototype_interior.png   # Scaled classroom interior & sensors
├── sketch_WTEC/
│   └── sketch_WTEC.ino              # Arduino main firmware
└── README.md
```

---

## 👥 Contributors

* **Doyeon Lim** ([@dylim-326](https://github.com/dylim-326)) — Biomedical Engineering, Hankuk Univ. of Foreign Studies
