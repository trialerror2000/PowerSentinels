# Power Sentinels
Wireless Sensor Networking(WSN) Implementation using ESP-NOW Protocol to detect anomalities in Power Consumption

## 🚩 The Problem
Unauthorized tapping into household power lines leads to unfair electricity bills for the resident and creates dangerous electrical hazards for the home. These illegal connections often bypass standard safety measures, increasing the risk of electrical fires and damaging sensitive household electronics.

---

## ✨ Key Features
*   **Precision Auditing:** Utilizes the **INA219 DC sensor** to monitor bus voltage, load current (mA), and power consumption (mW) with high accuracy.
*   **Fixed Detection Threshold:** Operates on a hardcoded **220mW limit** to ensure reliable, instant theft detection without false positives from sensor noise.
*   **3-State Intelligence:**
    1.  **Normal:** Smooth, slow-rotating RGB NeoPixel effect with live telemetry.
    2.  **Theft Alert:** High-intensity blinking red ring, synchronized buzzer, and **"BIJLI CHORI!!"** OLED warning.
    3.  **Offline:** Solid red indicator if a monitoring node loses connection to the dashboard.
*   **ESP-NOW Protocol:** Peer-to-peer communication between nodes and the dashboard, removing the need for a central Wi-Fi router for local alerts.
*   **IoT Oversight:** Real-time data logging to a remote dashboard for global monitoring from any location.

---

## 🛠️ Hardware Stack
| Component | Role |
| :--- | :--- |
| **ESP32 WROOM-32** | Central Gateway / Dashboard Brain |
| **Wemos D1 Mini** | Remote Leaf Node / Edge Sentinel |
| **INA219** | High-precision Power/Current Sensor |
| **SH1106 OLED** | 128x64 Onsite Diagnostic Display |
| **NeoPixel Ring** | 16-LED Visual Status Indicator |
| **Active Buzzer** | High-frequency Audio Alarm |

---

## ⚙️ Working Principles

### **WSN & Communication**
The prototype operates as a **Wireless Sensor Network (WSN)** where leaf nodes (Wemos D1 Mini) monitor individual household lines and report back to the parent gateway (ESP32). By utilizing **ESP-NOW**, a connectionless protocol, the boards communicate directly and rapidly. This ensures the system remains functional even without a local Wi-Fi network.

### **The State Machine**
The dashboard operates on three logic states:
*   **State 1 (Offline):** Triggered if no data is received from the node for 3 seconds.
*   **State 2 (Normal):** Active when power usage remains below the 220mW threshold.
*   **State 3 (Theft):** Triggered instantly when consumption exceeds 220mW.

### **Connectivity Requirements**
*   **Offline Security:** The core detection logic, buzzer, and LED alarms work **100% offline**, ensuring the home is protected even during internet outages.
*   **Online Features:** The **ESP32-CAM** photo capture and remote dashboard updates require an active internet connection to relay evidence and data to the cloud.

---

## 🚀 Current Progress
*   [x] High-precision INA219 integration for household monitoring.
*   [x] ESP-NOW communication established between Wemos D1 Mini and ESP32.
*   [x] 3-State UI logic (OLED + NeoPixel + Buzzer).
*   [x] Fixed threshold theft detection logic (220mW).
*   [x] Fully functional offline-resilient alarm system.

## 📅 Roadmap
*   [ ] **Evidence Capture:** Integrate an **ESP32-CAM** to take photos of tampering events.
*   [ ] **Proximity Sensing:** Trigger camera via IR or ToF sensors when motion is detected near the monitored line.
*   [ ] **Mobile Alerts:** Implement WhatsApp/Telegram notifications for instant remote theft reporting.

---
*Developed as a Computer Science and Engineering (CSE) prototype for regional technical exhibitions.*
