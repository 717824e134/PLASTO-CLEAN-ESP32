# PLASTO-CLEAN – ESP32 Safety & Motor Control Firmware

An embedded safety and motor-control prototype developed for the PLASTO-CLEAN tractor-mounted plastic removal system. The firmware monitors simulated sensor inputs, handles safety events, and controls motor and brush shutdown based on detected fault conditions.

---

## 🔗 Wokwi Simulation
**[▶️ Open Wokwi Simulation](https://wokwi.com/projects/472615262283921409)**
---

## ⚙️ Hardware & Pin Configuration

| Component | Function | Interface / Mode | ESP32 Pin |
| :--- | :--- | :--- | :--- |
| **Potentiometer** | Simulated Motor Load | ADC Input | `GPIO 34` |
| **HC-SR04** | Ultrasonic Trigger | Digital Output | `GPIO 5` |
| **HC-SR04** | Ultrasonic Echo | Digital Input | `GPIO 18` |
| **Push Button** | Emergency Stop (Active LOW) | Digital Input (Pull-up) | `GPIO 27` |
| **Relay Module** | Main Motor Power Cutoff | Digital Output | `GPIO 26` |
| **Servo Motor** | Plastic Collection Brush | PWM Output | `GPIO 12` |
| **Green LED** | Normal Status Indicator | Digital Output | `GPIO 25` |
| **Red LED** | Fault Alert Indicator | Digital Output | `GPIO 32` |
| **Active Buzzer** | Audible Fault Alarm | Digital Output | `GPIO 14` |

---

## 🛡️ Control Architecture & Safety Logic

The firmware follows a deterministic **Fault-priority architecture**, giving detected fault conditions precedence over normal actuator operation.

```text
       [ Read Sensors ]
    (Load, Distance, E-Stop)
               │
               ▼
      [ Evaluate Faults ]
       /               \
 [ Fault Detected ]     [ Normal ]
       │                    │
       ├─ Relay: OFF        ├─ Relay: ON
       ├─ Brush: STOP       ├─ Brush: RUN
       ├─ Red LED: ON       ├─ Green LED: ON
       └─ Buzzer: ON        └─ Buzzer / Red: OFF
