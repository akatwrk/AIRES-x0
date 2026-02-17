# AIRES-x0: Gestural Kinematic Robotic Control System

**AIRES-x0** (Artificial Intelligence & Robotic Engineering System - Experimental 0) is a high-fidelity, low-latency robotic manipulation platform. It translates human arm kinematics into 3-DOF (Degrees of Freedom) robotic motion using ESP-NOW wireless communication and Inertial Measurement Units (IMU).



---

##  Overview

Unlike traditional joystick-controlled arms, AIRES-x0 utilizes a wearable "Glove" interface. By leveraging **Sensor Fusion** and **Relative Kinematics**, the system creates a fluid, organic link between the user and the machine.

### Key Technical Features
* **Zero-Park Logic:** Built-in safety sequence that forces all servos to 0° on boot to prevent mechanical collisions or gear stripping.
* **Dual-Stage Filtering:** Implements Exponential Moving Averages (EMA) on both the Transmitter and Receiver to eliminate signal jitter and human hand tremors.
* **ESP-NOW Protocol:** Bypasses the overhead of standard Wi-Fi/Bluetooth, achieving near-instantaneous response times (~10ms latency).
* **Ergonomic Calibration:** A 3-second "Hand-Down" calibration sequence that sets the user's natural resting position as the system's global zero.

---

## 🛠 Hardware Architecture

### Master (The Glove)
* **Controller:** ESP32 DevKit V1
* **Sensor:** MPU6050 (6-Axis Accelerometer/Gyroscope)
* **Input:** Tactile Push Button (Logic Trigger)
* **Power:** 3.7V Li-Po (Portable)

### Slave (The Manipulator)
* **Controller:** ESP32 DevKit V1
* **Actuators:** 3x MG996R High-Torque Metal Gear Servos
* **Power:** External 5V 5A DC Power Supply (Common Grounded)



---

## 💻 Firmware Logic

The system utilizes a **Producer-Consumer** model over ESP-NOW.

1.  **Sensing:** The MPU6050 captures raw data, which is processed through a **Madgwick Filter** to calculate Yaw, Pitch, and Roll.
2.  **Mapping:** Euler angles are mapped from the human range (e.g., -90 to +90) to the mechanical servo range (0 to 90).
3.  **Smoothing:** $CurrentValue = (CurrentValue \times (1 - \alpha)) + (TargetValue \times \alpha)$
    *Where $\alpha$ represents the `FILTER_STRENGTH`.*



---

## 🔧 Installation & Setup

1.  **Clone the Repository:**
    ```bash
    git clone [https://github.com/akatwrk/AIRES-x0.git](https://github.com/akatwrk/AIRES-x0.git)
    ```
2.  **Configure MAC Address:**
    * Upload a MAC-finder sketch to your **Receiver** ESP32.
    * Copy the address into the `robotMAC[]` array in the **Transmitter** code.
3.  **Calibration Sequence:**
    * Power the system.
    * Hold the IMU perfectly still for 2 seconds (Gyro calibration).
    * Point your hand directly toward the ground for 3 seconds (Positional calibration).
4.  **Operate:** The arm will now track your wrist movement with fluid precision.

---
