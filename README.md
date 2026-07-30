# IoT Course – Challenges & Homework (Politecnico di Milano, ANTLab)

This repository contains the code, reports and solutions developed for the four IoT assignments of the course:

- **Challenge #1** – Wokwi and Power Consumption
- **Challenge #2** – Packet Sniffing
- **Challenge #3** – Node-RED and LoRaWAN
- **Homework** – 802.15.4, BLE Localization, RFID

---

## 🧩 Challenge #1 – Wokwi and Power Consumption

**Goal:** design a commercial-style IoT motion sensor for a smart home, simulated in [Wokwi](https://wokwi.com/), that:
- detects human motion and measures ambient light level (illuminance);
- sends the data to a central **ESP32 sink node** via **ESP-NOW**, using the format:
  - `MOTION_DETECTED-LUMINOSITY:ZZZ`
  - `MOTION_NOT_DETECTED-LUMINOSITY:ZZZ`
- enters **Deep Sleep** for `X` seconds between each sensing/transmission cycle.

**Work done:**
1. Implemented the sensor node firmware (motion detection + light sensing + ESP-NOW transmission + Deep Sleep cycle) in Wokwi.
2. Estimated the average power consumption of each operating state (Deep Sleep, Idle, Sensor reading, Transmission) using the provided power-consumption CSV files, combined with timing measurements (`micros()`) taken from the simulation.
3. Computed the energy consumed in one full operating cycle and estimated the **battery lifetime** given a battery energy budget `Y`.
4. Proposed and implemented improvements to reduce energy consumption (e.g. reducing transmission power, minimizing active time), and re-evaluated the energy consumption after the optimization.
5. Solved the theoretical **energy trade-off exercise** in a multi-hop WSN topology (direct vs. multi-hop transmission energy comparison).

---

## 🧩 Challenge #2 – Packet Sniffing

**Goal:** analyze IoT network traffic captured in two PCAP files (`A.pcapng`, `B.pcapng`) containing **CoAP** and **MQTT/MQTT-SN** traffic, using Wireshark and/or custom scripts.

**Work done – Part 1 (Challenge Questions CQ1–CQ8):**
- Identified NON-Confirmable DELETE requests to the `coap.me` server and checked their outcome.
- Counted CoAP resources receiving an equal number of unique unsuccessful CON POST/PUT requests.
- Counted unique CoAP Observe notifications for `/dining_room/temperature` and identified redundant/useless ones.
- Counted MQTT-SN messages received by clients from the local broker (port 1885).
- Analyzed MQTT last-will messages derived from wildcard subscriptions.
- Identified MQTT clients erasing retained values on the public HiveMQ broker and their client-ID length.
- Counted MQTT subscribe requests to the local broker using topics with at least two wildcards.
- Produced a histogram comparing the **topic depth distribution** of PUBLISH messages directed to the local broker across both captures.

**Work done – Part 2 (Exercise EQ1–EQ2):**
- Computed the total communication energy (in µJ) consumed over one hour by a battery-powered temperature sensor and actuator, comparing **direct CoAP communication** vs. **MQTT communication through a gateway** (QoS 1, CON messages, retransmission parameters, connect/disconnect overhead, etc.).
- Discussed which protocol is more energy-efficient for long-term operation when the actuator's duty cycle changes (wake-up every 30 minutes), justifying the choice in terms of protocol parameters (clean session, retain, observe, block-transfer) and the optimized performance metric.

---

## 🧩 Challenge #3 – Node-RED and LoRaWAN

**Goal:** build a Node-RED flow to process a Zigbee packet-capture dataset (`challenge3.csv`) and interact with local MQTT broker(s) and ThingSpeak; then solve a LoRaWAN sizing exercise.

**Work done – Part 1 (Node-RED flow):**
1. **ID Generator branch:** periodically (every 1 s) publishes a random `id` (0–30000) with a UNIX timestamp to the local Mosquitto broker (`challenge3/id_generator`, port 1884), logging every message to `id_log.csv`.
2. **Subscriber branch:**
   - Subscribes to `challenge3/id_generator`, computes `N = ID mod 5218`, and retrieves the packet with `Packet Number = N` from `challenge3.csv`.
   - If the retrieved packet contains a **ZBEE_ZCL layer**, publishes a structured JSON message (with timestamp, id, source/destination addresses in hex, and command payload) to a topic named after the ZigBee source device, rate-limited to 10 messages/minute.
   - If the packet's Command String includes **RMS Current**, **RMS Voltage**, or **Active Power** attributes, extracts and logs the matched attribute/status/data-type/value tuples to `filtered_elems.csv`, and plots RMS Current and RMS Voltage on two Node-RED dashboard charts.
   - If the packet is a **Link Status (0x08)** message, tracks and updates the outgoing link cost per ZigBee source/destination pair, saving the final result to `outgoing_cost.csv`.
   - All other packets are ignored; the flow stops after exactly 200 processed ID messages.
3. Sent the outgoing costs (sorted by destination address) for the smallest source address to a public **ThingSpeak** channel via HTTP API, respecting a rate limit of 1 message/20 s.

**Work done – Part 2 (LoRaWAN Exercise EQ1–EQ2):**
- Determined the largest LoRa **Spreading Factor (SF)** guaranteeing at least a 75% packet success rate for a network of 40 nodes transmitting at λ = 2 packets/minute (868 MHz, BW = 125 kHz), using the [TTN airtime calculator](https://www.thethingsnetwork.org/airtime-calculator) and a simplified collision model.
- Discussed the most appropriate corrective action (reduce node count / move nodes closer / increase SF) when field-measured success rates are lower than expected and non-uniform across nodes.

---

## 📘 Homework

### Part 1 – IEEE 802.15.4
Analysis of an ESP32-based vibration monitoring node operating in **beacon-enabled mode (CFP only)**. Given a Poisson-distributed vibration-peak detection process, the assignment computes:
- the PMF of the output data rate depending on the number of detected peaks;
- a consistent **CFP slot assignment**;
- `Ts`, `Nslots`, `Tactive`, `Tinactive` and the resulting **duty cycle**;
- the maximum number of additional nodes that can be added while keeping the duty cycle below 10%.

### Part 2 – BLE Localization
Redesign of a naive periodic-beaconing BLE localization system (used to track dental prostheses of Alzheimer patients in a nursing home) to **reduce battery consumption while preserving localization accuracy**. The proposed solution includes:
- an event-driven / adaptive BLE beaconing strategy;
- additional sensors (e.g. motion/accelerometer) to trigger beaconing only when needed;
- improved firmware pseudocode;
- real commercial components suitable for the proposed sensing logic;
- a quantitative energy-saving estimate compared to the original fixed 10 s periodic beaconing.

### Part 3 – RFID
Study of an RFID system based on **Dynamic Frame Slotted ALOHA** with N = 4 tags:
- computation of the overall collision-resolution efficiency **η** for different initial frame sizes (`r1 = 1…6`), assuming the frame size is correctly re-sized to the backlog after the first frame;
- a plot of **η vs. initial frame size r1**;
- discussion of which initial frame size(s) maximize η.

---

## 🛠️ Tools & Technologies

- [Wokwi](https://wokwi.com/) – ESP32 circuit/firmware simulation
- [Wireshark](https://www.wireshark.org/) – PCAP/CoAP/MQTT/MQTT-SN traffic analysis
- [Node-RED](https://nodered.org/) – flow-based IoT data processing
- [Mosquitto](https://mosquitto.org/) – local MQTT broker
- [ThingSpeak](https://thingspeak.com/) – IoT data visualization platform
- Python (data processing / plotting for CQ8, RFID efficiency plot, etc.)
- IEEE 802.15.4, BLE, RFID (Dynamic Frame ALOHA), CoAP, MQTT/MQTT-SN, LoRaWAN protocols

---

## 👥 Authors
- **[Mattia Brianti](https://github.com/MattiaBrianti)**
- **[Alex Hathaway](https://github.com/Alexhath)**
