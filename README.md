# Secure IoT Smart Farming System (ESP32 + ECC)

![Hardware](https://img.shields.io/badge/Hardware-ESP32-green)
![Platform](https://img.shields.io/badge/Platform-Arduino_IDE-blue)
![Security](https://img.shields.io/badge/Security-ECC_%2B_ChaCha20-red)
![License](https://img.shields.io/badge/License-MIT-orange)

A multi-node, secure IoT architecture designed for smart farming applications. This project implements **End-to-End Encryption** using **Elliptic Curve Cryptography (ECC)** and **ChaCha20-Poly1305** to protect sensor data transmitted via **ESP-NOW**. The system acts as a secure bridge to the **Blynk Cloud**.

## Key Features

* **Hybrid Encryption:** Uses `Curve25519` for ECDH key exchange and `ChaCha20-Poly1305` for authenticated payload encryption.
* **High Performance:** Utilizes **ESP-NOW** (Connectionless Wi-Fi) for ultra-low latency and power efficiency on sensor nodes.
* **Centralized Gateway:** A Collector node aggregates data and forwards it to the cloud via TLS/SSL.
* **Dynamic Configuration:** Integrated **WiFiManager** allows easy Wi-Fi credential setup without hardcoding.
* **Multi-Node Support:** Designed to handle multiple sender nodes (Node 1 & Node 2) with distinct identity keys.

## System Topology

The system consists of Sensor Nodes communicating securely with a central Collector, which acts as a Gateway to the Internet.

```mermaid
graph TD
    %% Node Styles
    classDef sensor fill:#e1f5fe,stroke:#01579b,stroke-width:2px;
    classDef gateway fill:#fff9c4,stroke:#fbc02d,stroke-width:2px;
    classDef cloud fill:#f3e5f5,stroke:#4a148c,stroke-width:2px,stroke-dasharray: 5 5;

    subgraph "Local Sensor Network (ESP-NOW)"
        N1[NODE 1<br/>(Sender)]:::sensor
        N2[NODE 2<br/>(Sender)]:::sensor
    end

    G[COLLECTOR<br/>(Gateway)]:::gateway
    R((WiFi Router))
    B(Blynk Cloud):::cloud
    App[Mobile Dashboard]

    %% Data Flow
    N1 -- "Encrypted (ECC + ChaCha20)" --> G
    N2 -- "Encrypted (ECC + ChaCha20)" --> G
    
    G -- "TLS / SSL (Port 443)" --> R
    R -- "Internet" --> B
    B <--> App
```

## Hardware & Wiring

### Components Required
* 3x ESP32 Development Boards (DOIT DevKit V1 recommended)
* 2x DHT11 Temperature & Humidity Sensors
* 2x YL-69 Capacitive Soil Moisture Sensors
* Power Source (5V USB / Li-Po Battery)

### Pinout Configuration (Sender Nodes)

| Component | Pin Name | ESP32 Pin | Function |
| :--- | :--- | :--- | :--- |
| **DHT11** | VCC | 3V3 | Power (Always ON) |
| | GND | GND | Ground |
| | DATA | **GPIO 4** | Temp/Hum Data |
| **Soil Sensor** | VCC | **GPIO 5** | Power Control (Anti-Corrosion) |
| | GND | GND | Ground |
| | A0 (Analog) | **GPIO 34** | Moisture Data (Input Only) |

### Visual Schema

```text
       ESP32 BOARD (SENDER NODE)
      +-------------------------+
      |                         |
      |                    3V3 -+---------> DHT11 (VCC)
      |                    GND -+--+------> DHT11 (GND)
      |                     D4 -+--|------> DHT11 (DATA)
      |                         |  |
      |                     D5 -+--|------> Soil Sensor (VCC)
      |                    D34 -+--|------> Soil Sensor (A0)
      |                    GND -+--+------> Soil Sensor (GND)
      |                         |
      +-------------------------+
```

## Security Mechanism

This project moves beyond standard "plaintext" IoT by implementing a cryptographic handshake:

1.  **Identity Generation:** Each node generates a persistent `Private Key` and `Public Key` pair.
2.  **Shared Secret (ECDH):** The Sender and Collector calculate a shared secret mathematically using `Curve25519`. The Private Key is **never** transmitted.
    > *SharedSecret = (Priv_Sender * Pub_Collector) = (Priv_Collector * Pub_Sender)*
3.  **Payload Encryption:** Sensor data (`Struct`) is encrypted using **ChaCha20-Poly1305** with a unique random IV (Nonce) for every packet.
4.  **Integrity Check:** The Poly1305 Auth Tag ensures data has not been tampered with during transmission.

## Installation Guide

### 1. Library Dependencies
Install these libraries via Arduino IDE Library Manager:
* `Crypto` by **Rhys Weatherley** (Critical for ECC/ChaCha)
* `Blynk` by **Volodymyr Shymanskyy**
* `WiFiManager` by **tzpu**
* `DHT sensor library` by **Adafruit**

### 2. Generate Keys
1.  Flash `src/KeyGenerator/KeyGenerator.ino` to your Node 1, Node 2, and Collector boards individually.
2.  Open Serial Monitor.
3.  **Save the output!** These are your unique cryptographic identities.

### 3. Setup Collector (Gateway)
1.  Open `src/CollectorNode/CollectorNode.ino`.
2.  Update `myPrivKey` with the Collector's Private Key.
3.  Update `node1PubKey` and `node2PubKey` with the Nodes' Public Keys.
4.  Flash the board.
5.  **Important:** Note the `WiFi Channel` printed on the Serial Monitor.

### 4. Setup Senders (Nodes)
1.  Open `src/SenderNode_1/SenderNode_1.ino` (or Node 2).
2.  Update `WIFI_CHANNEL` to match the Collector's channel.
3.  Update `collectorPubKey` with the Collector's Public Key.
4.  Update `myPrivKey` with the Node's Private Key.
5.  Flash the board.

## Blynk Setup

Create a Template in Blynk Console with the following Datastreams:

| Node | Data | Virtual Pin | Data Type | Min/Max |
| :--- | :--- | :--- | :--- | :--- |
| **Node 1** | Temperature | V0 | Double | 0 - 100 |
| | Humidity | V1 | Double | 0 - 100 |
| | Soil Moisture | V2 | Integer | 0 - 100 |
| **Node 2** | Temperature | V3 | Double | 0 - 100 |
| | Humidity | V4 | Double | 0 - 100 |
| | Soil Moisture | V5 | Integer | 0 - 100 |

## 📄 License

This project is open-source and licensed under the [MIT License](LICENSE).