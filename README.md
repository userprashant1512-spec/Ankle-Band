# SafeStepper – Smart Vibration Assistive Ankle Band (BLE + IoT)

SafeStepper is a **smart assistive wearable system** designed to help users—especially visually or hearing-impaired individuals—gain real-time awareness of their surroundings using **vibration feedback**.

The system uses an **ESP32 with BLE (Bluetooth Low Energy)** to communicate with a **custom web application**, providing alerts for obstacles, fire, loud sounds, and directional hazards through vibration motors mounted on an ankle band.

---

## 🚀 Features

- 🔊 **Sound Detection (Clap / Loud Noise)**
  - Detects loud sounds near laptop or phone microphone
  - Sends BLE alert to ESP32
  - Triggers vibration for 3 seconds

- 📏 **Ultrasonic Obstacle Detection**
  - Detects obstacles in front
  - Vibrates both motors for 5 seconds

- 🔥 **Flame / Fire Detection**
  - Detects fire using flame sensor (active-LOW)
  - Strong vibration alert for 5 seconds

- ⬅➡ **Directional IR Detection**
  - Left IR sensor → Left motor vibrates for 2 seconds
  - Right IR sensor → Right motor vibrates for 2 seconds

- 🔋 **Battery Monitoring**
  - Displays battery percentage in the web app

- 🔵 **BLE Connection Indicator**
  - Blue LED ON when BLE is connected
  - LED OFF when disconnected

- 🌐 **Custom Web App Dashboard**
  - Live sensor data display
  - BLE connect / disconnect
  - Sound detection trigger
  - Clean mobile-friendly UI

---

## 🧠 System Architecture

Sensors (Ultrasonic / IR / Flame / Battery)
↓
ESP32
(BLE Peripheral)
↓
Web Bluetooth API
↓
Custom Web App
↓
Vibration Feedback


---

## 🛠️ Hardware Components

- ESP32 Development Board  
- Ultrasonic Sensor (HC-SR04 or equivalent)  
- Flame Sensor (Digital Output)  
- IR Obstacle Sensors (Left & Right)  
- Vibration Motors (with transistor driver)  
- Battery + Voltage Divider / 25V Sensor  
- NPN Transistor (BC547 / 2N2222)  
- Diode (1N4007)  
- Ankle Band / Wearable Mount  

---

## 💻 Software & Technologies Used

- **ESP32 Arduino Framework**
- **Bluetooth Low Energy (BLE)**
- **Web Bluetooth API**
- **Web Audio API (Sound Detection)**
- **HTML, CSS, JavaScript**
- **Arduino IDE**

---

## 📡 BLE Communication

- **Service UUID:** `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
- **RX Characteristic:** Receives commands (`ALERT`, `STOP`)
- **TX Characteristic:** Sends sensor data in real time

Example BLE payload:
