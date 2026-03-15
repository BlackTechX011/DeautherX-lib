/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#include "DeautherSettings.h"
#include "../utils/DeautherMac.h"
#include "../utils/DeautherStrHelper.h"

namespace DSettingsManager {

static DSettings _settings;

void load() {
    EEPROM.begin(sizeof(DSettings) + 8);
    EEPROM.get(0, _settings);
    if (_settings.magic != 0xDE017A) {
        Serial.println(F("[Settings] EEPROM not initialized, loading defaults"));
        reset();
    } else {
        Serial.println(F("[Settings] Loaded from EEPROM"));
    }
}

void save() {
    EEPROM.put(0, _settings);
    EEPROM.commit();
}

void reset() {
    _settings = DSettings{}; // reset to defaults
    save();
    Serial.println(F("[Settings] Reset to defaults"));
}

DSettings& get() { return _settings; }

void set(const DSettings& s) {
    _settings = s;
    _settings.magic = 0xDE017A;
}

void print() {
    Serial.println(F("╔══════════════════════════════════════╗"));
    Serial.println(F("║           SETTINGS                   ║"));
    Serial.println(F("╠══════════════════════════════════════╣"));
    Serial.print(F("║  Attack All CH:   ")); Serial.println(_settings.attack_all_ch ? F("Yes") : F("No"));
    Serial.print(F("║  Random TX:       ")); Serial.println(_settings.random_tx ? F("Yes") : F("No"));
    Serial.print(F("║  Deauths/target:  ")); Serial.println(_settings.deauths_per_target);
    Serial.print(F("║  Reason code:     ")); Serial.println(_settings.deauth_reason);
    Serial.print(F("║  Beacon interval: ")); Serial.println(_settings.beacon_interval_1s ? F("1s") : F("100ms"));
    Serial.print(F("║  Probes/SSID:     ")); Serial.println(_settings.probe_frames_per_ssid);
    Serial.print(F("║  Send deauth:     ")); Serial.println(_settings.send_deauth ? F("Yes") : F("No"));
    Serial.print(F("║  Send disassoc:   ")); Serial.println(_settings.send_disassoc ? F("Yes") : F("No"));
    Serial.print(F("║  Channel:         ")); Serial.println(_settings.channel);
    Serial.print(F("║  Scan CH time:    ")); Serial.print(_settings.scan_channel_time); Serial.println(F("ms"));
    Serial.print(F("║  Scan timeout:    ")); Serial.print(_settings.scan_timeout); Serial.println(F("ms"));
    Serial.print(F("║  ET SSID:         ")); Serial.println(_settings.et_ssid);
    Serial.print(F("║  ET Channel:      ")); Serial.println(_settings.et_channel);
    Serial.println(F("╚══════════════════════════════════════╝"));
}

String toJSON() {
    String j = F("{");
    j += F("\"attack_all_ch\":"); j += DStr::boolean(_settings.attack_all_ch); j += ',';
    j += F("\"random_tx\":"); j += DStr::boolean(_settings.random_tx); j += ',';
    j += F("\"deauths_per_target\":"); j += _settings.deauths_per_target; j += ',';
    j += F("\"deauth_reason\":"); j += _settings.deauth_reason; j += ',';
    j += F("\"beacon_interval_1s\":"); j += DStr::boolean(_settings.beacon_interval_1s); j += ',';
    j += F("\"probe_frames_per_ssid\":"); j += _settings.probe_frames_per_ssid; j += ',';
    j += F("\"channel\":"); j += _settings.channel; j += ',';
    j += F("\"scan_channel_time\":"); j += _settings.scan_channel_time; j += ',';
    j += F("\"scan_timeout\":"); j += _settings.scan_timeout;
    j += '}';
    return j;
}

} // namespace DSettingsManager
