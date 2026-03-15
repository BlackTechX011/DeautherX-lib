/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#include "DeautherSSIDList.h"

DeautherSSIDList::DeautherSSIDList() {
    _entries = new Entry[MAX_ENTRIES];
    _count   = 0;
}

DeautherSSIDList::~DeautherSSIDList() {
    delete[] _entries;
}

bool DeautherSSIDList::add(const char* ssid, bool wpa2) {
    if (!ssid || _count >= MAX_ENTRIES) return false;
    // Deduplicate
    for (uint8_t i = 0; i < _count; i++) {
        if (strncmp(_entries[i].ssid, ssid, MAX_LEN) == 0) return false;
    }
    uint8_t len = strlen(ssid);
    if (len > MAX_LEN) len = MAX_LEN;
    memcpy(_entries[_count].ssid, ssid, len);
    _entries[_count].ssid[len] = '\0';
    _entries[_count].wpa2      = wpa2;
    _count++;
    return true;
}

void DeautherSSIDList::remove(uint8_t index) {
    if (index >= _count) return;
    for (uint8_t i = index; i < _count - 1; i++) {
        _entries[i] = _entries[i + 1];
    }
    _count--;
}

void DeautherSSIDList::clear() { _count = 0; }

uint8_t     DeautherSSIDList::count()            const { return _count; }
bool        DeautherSSIDList::empty()            const { return _count == 0; }
const char* DeautherSSIDList::getSSID(uint8_t i) const { return i < _count ? _entries[i].ssid : ""; }
bool        DeautherSSIDList::getWPA2(uint8_t i) const { return i < _count && _entries[i].wpa2; }

void        DeautherSSIDList::begin()    { _iter_pos = 0; _iter_ready = true; }
const char* DeautherSSIDList::iterateSSID() {
    const char* r = (_iter_pos < _count) ? _entries[_iter_pos].ssid : "";
    _iter_pos++;
    return r;
}
bool DeautherSSIDList::iterateWPA2() {
    return (_iter_pos > 0 && _iter_pos - 1 < _count) ? _entries[_iter_pos - 1].wpa2 : false;
}
bool DeautherSSIDList::available() const { return _iter_ready && _iter_pos < _count; }

void DeautherSSIDList::print() const {
    for (uint8_t i = 0; i < _count; i++) {
        Serial.print(F("  "));
        Serial.print(i);
        Serial.print(F("  "));
        Serial.print(_entries[i].ssid);
        Serial.println(_entries[i].wpa2 ? F("  [WPA2]") : F("  [Open]"));
    }
}
