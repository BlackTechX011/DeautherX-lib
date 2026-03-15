s

# 📡 DeautherX-lib

**The Unified ESP8266 Wi-Fi Security & Testing Library**

[![Version](https://img.shields.io/badge/version-0.1.0-blue.svg?style=for-the-badge)](https://github.com/BlackTechX011/DeautherX-lib)
[![Platform](https://img.shields.io/badge/platform-ESP8266-green.svg?style=for-the-badge)](https://www.espressif.com/en/products/socs/esp8266)
[![License](https://img.shields.io/badge/license-MIT-yellow.svg?style=for-the-badge)](LICENSE)

</div>

<br>

> ⚠️ **DISCLAIMER:** This software is provided for **educational purposes and ethical security testing only**. You may only use this on networks you own or have explicit written permission to test. The authors and maintainers assume no liability for misuse, damage, or legal consequences. By using this library, you accept full responsibility for your actions.

**DeautherX-lib** extracts the core engine of the popular [ESP8266 Deauther](https://github.com/SpacehuhnTech/ESP8266_Deauther) series and [DeautherX](https://github.com/BlackTechX011/DeautherX), condensing them into a single, high-performance C++ library. 

By stripping away all hardcoded UI elements (Web UI, OLED displays, CLI), this library provides a clean, modular API. It empowers developers to build custom Wi-Fi auditing tools, headless security nodes, or integrate Deauther functionality into entirely new user interfaces.

---

## 📑 Table of Contents
- [✨ Features](#-features)
- [⚙️ Requirements](#️-requirements)
- [📦 Installation](#-installation)
- [🚀 Quick Start](#-quick-start)
- [🏗 Architecture Overview](#-architecture-overview)
- [📚 Examples](#-examples)
- [🗺 Roadmap](#-roadmap)
- [📄 License](#-license)

---

## ✨ Features

- 🧩 **UI-Agnostic Design:** Pure background functionality. Build your own Display, Serial CLI, or Web interface without fighting the core logic.
- 📡 **Advanced Scanning Engine:** Rapid Access Point (AP) and Station scanning, RSSI tracking, and continuous handshake/association capture.
- ⚔️ **Targeted Deauthentication:** Execute precise deauthentication attacks against specific targets.
- 📶 **Beacon Flooding:** Generate and broadcast custom SSID beacon spam.
- 🎭 **Evil Twin & Captive Portal:** Configurable Evil Twin attacks with forced captive portals and custom HTML injection.
- 🛑 **Rogue AP:** Easily spin up fully functional Rogue Access Points (open or closed).
- 💾 **Secure Local Storage:** Automated management of captured data (credentials, probe requests) utilizing the ESP8266 `LittleFS` file system.
- 🛠 **Highly Customizable:** Supply your own phishing HTML templates and scripts directly via the ESP8266 file system.

---

## ⚙️ Requirements

To use this library, ensure your development environment meets the following specifications:

- **Hardware:** ESP8266 (e.g., NodeMCU, Wemos D1 Mini)
- **Framework:** Arduino Core for ESP8266 (v3.1.2 or higher recommended)
- **Dependencies:** 
  - `ESP8266WiFi`
  - `DNSServer`
  - `ESP8266WebServer`
  - `ESP8266mDNS`
  - `LittleFS`

---

## 📦 Installation

### Option A: Arduino IDE
1. Download this repository as a `.zip` file.
2. Open the Arduino IDE.
3. Navigate to **Sketch** > **Include Library** > **Add .ZIP Library...**
4. Select the downloaded `.zip` file.

### Option B: PlatformIO (Recommended)
Add the following line to your `platformio.ini` file under `lib_deps`:
```ini
lib_deps =
    https://github.com/BlackTechX011/DeautherX-lib.git
```

---

## 🚀 Quick Start

Initialize the core library features by including the main header file. The library relies on a non-blocking architecture, requiring `DeautherLib::update()` to be called continuously.

```cpp
#include <Arduino.h>
#include <DeautherLib.h>

void setup() {
    Serial.begin(115200);

    // Initialize the library and subsystems
    DeautherLib::begin();
    
    Serial.println("DeautherX-lib initialized successfully.");
}

void loop() {
    // Keep the background processes running (scanning, attacks, portals)
    DeautherLib::update();
    
    // Your custom UI or logic goes here
}
```

---

## 🏗 Architecture Overview

The library is modular by design. You can interact with specific subsystems based on your needs:

| Subsystem | Class | Description |
| :--- | :--- | :--- |
| **Scanner** | `DeautherScanner` | Handles AP, Station, RSSI discovery, and Promiscuous Sniffer modes. |
| **Attack** | `DeautherAttack` | Manages active Deauthentication and Beacon flood routines. |
| **Evil Twin** | `DeautherEvilTwin` | Spawns forced captive portals and manages credential harvesting. |
| **Rogue AP** | `DeautherRogueAP` | Handles basic open/closed rogue access point configurations. |

---

## 📚 Examples

Check the [`examples/`](examples/) directory for ready-to-flash sketches that demonstrate the library's capabilities:

- 💻 **`CLIDeauther`**: A fully-featured Serial Command Line Interface. Combines scanning, attacks, and evil twin configurations into an interactive prompt.
- 🌊 **`BeaconFlood`**: A minimal implementation demonstrating how to execute a multi-SSID beacon flood attack.
- 🎯 **`ScanAndAttack`**: Demonstrates the programmatic, headless flow of hunting for specific Wi-Fi targets and engaging them automatically.

---

## 🗺 Roadmap

- [ ] Write comprehensive API documentation (Doxygen / GitHub Wiki).
- [ ] Implement additional attack vectors and protocol support.
- [ ] Optimize memory footprint for heavy AP environments.
- [ ] Open for community ideas—[Submit a Feature Request!](https://github.com/BlackTechX011/DeautherX-lib/issues/new)

---

## 🤝 Contributing

Contributions, issues, and feature requests are highly welcome! Feel free to check the [issues page](https://github.com/BlackTechX011/DeautherX-lib/issues). If you want to contribute code, please fork the repository and submit a pull request.

---

## 📄 License

This project is licensed under the **MIT License**. See the [LICENSE](LICENSE) file for full details. 

*Credits to the original creators of [ESP8266 Deauther](https://github.com/SpacehuhnTech/ESP8266_Deauther) and [DeautherX](https://github.com/BlackTechX011/DeautherX) for the foundational research and code.*