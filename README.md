# CSC473-Final-Destination
# Santa’s Smart Delivery Car

Created by **Shrishtika Bajracharya** and **Bivushi Basnet**.

A Christmas-themed autonomous delivery car built with Arduino and integrated sensors, adheres to traffic rules.

A sensor-driven Arduino car that operates in Manual, Automatic Maze, and Emergency Stop modes.
The system also includes a synchronized LCD taskboard and 7-segment scoreboard, all controlled through IR-remote–based communication — all wrapped inside a festive gift-delivery theme.

---

## 📌 Overview

Santa’s Smart Delivery Car navigates a miniature town, delivers gifts, follows lanes, senses obstacles, and syncs with multiple external displays. The system supports three modes:

* **Manual Mode** – IR-remote driving with safety features
* **Autonomous Mode** – Line following, obstacle detection, auto-steering
* **Emergency Stop Mode** – Immediate halt + red LED strip indicator

---

## 🛠 Hardware Used

From *Core Hardware* :

* **Arduino UNO**
* **IR line sensors** (3-array)
* **Ultrasonic sensor** (HC-SR04)
* **Motor driver module**
* **Buzzer + battery pack**
* **LED Strip (WS2812B)** for mode indication
* **7-segment scoreboard**
* **LCD taskboard**
* **IR receiver + IR remote**

---

# 🚗 Modes & Features

## **Manual Mode (Green LED)**

* IR-remote controlled driving
* Double beep when crossing zebra lines
* Auto-stop on all-black tile (delivery point)
* Ultrasonic warning when too close to obstacles
* Timeout → safety stop

---

## **Autonomous Mode (Blue LED)**

* Dynamic line-following across any dark surface
* Auto-calibrating threshold for reflective floors
* Smooth turn logic
* Obstacle detection + stop
* No manual steering required

---

## **Interactive Displays**

### **Task LCD Board**

* Shows mode, task, and a **30-second countdown**

### **7-Segment Scoreboard**

* Tracks score **00 → 99**
* Fully IR-synced with the car and taskboard

---

# 🔧 Challenges & Solutions

| **Challenge**                                   | **Solution**                                              |
| ----------------------------------------------- | --------------------------------------------------------- |
| Unstable sensors on reflective surfaces         | Adaptive dynamic threshold                                |
| No communication between car & scoreboard       | Switched fully to IR synchronization                      |
| Battery power drops                             | Simple power-button scoring method                        |
| Could not sync traffic light (no color sensors) | Internal syncing using user color codes (early prototype) |
| Joystick unsupported (no wireless module)       | Replaced with reliable IR remote                          |

---

# 🚀 Future Enhancements

* PID line-following
* Automatic maze solving
* Real traffic light detection
* Bluetooth or nRF wireless control
* Larger map with intersections

---

# 🏁 Final Achievements

### **Functional Car**

* Manual + autonomous modes
* Smooth acceleration/braking
* Ultrasonic collision prevention

### **Synced Displays**

* Real-time IR-based synchronization
* Stable 30-second task timer

### **Stable Line Following**

* Adaptive thresholding
* Works even on varied surfaces

### **Interactive Game**

* Festive delivery challenge
* Clear visual countdown + feedback

---


# 📧 Contact

Any questions? Reach out:

* **[sbajrac2@oswego.edu](mailto:sbajrac2@oswego.edu)**
* **[bbasnet2@oswego.edu](mailto:bbasnet2@oswego.edu)**

---
