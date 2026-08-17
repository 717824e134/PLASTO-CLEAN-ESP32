# 🚜 PLASTO-CLEAN – ESP32 Safety & Motor Control

An ESP32-based control and safety prototype for the **PLASTO-CLEAN
tractor-mounted plastic removal system**, developed and tested using
**Wokwi simulation**.

## 🔎 Overview

PLASTO-CLEAN is designed to assist in removing plastic waste trapped
around fences, vegetation and roadside areas.

The embedded control system monitors simulated motor load, obstacles
and emergency-stop conditions.

When an unsafe condition is detected, the system automatically stops
the brush and motor and activates warning indicators.

## ⚙️ Key Features

- Motor overload detection
- Ultrasonic obstacle detection
- Emergency stop
- Automatic relay-based motor shutdown
- Servo-based brush control
- Buzzer fault alert
- Red/green status indicators
- Automatic recovery after fault clearance
- Serial monitoring of system parameters

## 🧠 System Logic

```text
                 ┌──────────────────┐
                 │   ESP32 START    │
                 └────────┬─────────┘
                          │
                          ▼
                 ┌──────────────────┐
                 │ Read Sensors     │
                 │ Load + Distance │
                 │ + Emergency     │
                 └────────┬─────────┘
                          │
                          ▼
                 ┌──────────────────┐
                 │ Fault Detected?  │
                 └───────┬───┬──────┘
                         │   │
                       NO│   │YES
                         │   │
                         ▼   ▼
                  ┌────────┐ ┌──────────────┐
                  │ Motor  │ │ Motor OFF    │
                  │ ON     │ │ Brush STOP   │
                  │        │ │ Buzzer ON    │
                  │ Green  │ │ Red LED ON   │
                  │ LED ON │ │              │
                  └────────┘ └──────────────┘
