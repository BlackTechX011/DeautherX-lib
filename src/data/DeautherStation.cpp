/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#include "DeautherStation.h"
#include "../utils/DeautherMac.h"
#include "../utils/DeautherStrHelper.h"
#include "../utils/DeautherVendor.h"

DeautherStation::DeautherStation(const uint8_t* mac, DeautherAP* ap)
    : _ap(ap), _probe_count(0)
{
    if (mac) memcpy(_mac, mac, 6);
    else     memset(_mac, 0, 6);
    memset(_probes, 0, sizeof(_probes));
}

DeautherStation::~DeautherStation() {
    for (uint8_t i = 0; i < _probe_count; i++) {
        delete[] _probes[i];
    }
}

const uint8_t* DeautherStation::getMAC()     const { return _mac; }
String         DeautherStation::getMACStr()  const { return DMac::toStr(_mac); }
DeautherAP*    DeautherStation::getAP()      const { return _ap; }
int8_t         DeautherStation::getRSSI()    const { return _rssi; }
uint32_t       DeautherStation::getPackets() const { return _pkts; }
bool           DeautherStation::isSelected() const { return _selected; }

String DeautherStation::getAPSSID() const {
    return _ap ? _ap->getSSIDStr() : String();
}

String DeautherStation::getAPBSSID() const {
    return _ap ? _ap->getBSSIDStr() : String(F("—"));
}

uint8_t DeautherStation::getChannel() const {
    return _ap ? _ap->getChannel() : 0;
}

String DeautherStation::getVendor() const {
    return DVendor::getName(_mac);
}

String DeautherStation::getProbesStr() const {
    String r;
    for (uint8_t i = 0; i < _probe_count; i++) {
        if (i) r += ',';
        r += _probes[i];
    }
    return r;
}

void DeautherStation::setAP(DeautherAP* ap) { _ap = ap; }

void DeautherStation::addProbe(const char* ssid, uint8_t len) {
    if (!ssid || len == 0 || _probe_count >= MAX_PROBES) return;
    // Deduplicate
    for (uint8_t i = 0; i < _probe_count; i++) {
        if (strncmp(_probes[i], ssid, len) == 0) return;
    }
    char* copy = new char[len + 1];
    memcpy(copy, ssid, len);
    copy[len] = '\0';
    _probes[_probe_count++] = copy;
}

void DeautherStation::recordPacket(int8_t rssi) {
    _pkts++;
    // Exponential moving average for RSSI
    _rssi = (_rssi == -127) ? rssi : (int8_t)((_rssi * 3 + rssi) / 4);
}

void DeautherStation::select()         { _selected = true;  }
void DeautherStation::deselect()       { _selected = false; }
void DeautherStation::toggleSelected() { _selected = !_selected; }

String DeautherStation::toJSON(int id) const {
    String j = F("{");
    if (id >= 0) { j += F("\"id\":"); j += id; j += ','; }
    j += F("\"mac\":\"");     j += getMACStr();    j += F("\",");
    j += F("\"ap_bssid\":\"");j += getAPBSSID();   j += F("\",");
    j += F("\"ap_ssid\":\""); j += DStr::jsonEscape(getAPSSID()); j += F("\",");
    j += F("\"rssi\":");      j += _rssi;           j += ',';
    j += F("\"pkts\":");      j += _pkts;           j += ',';
    j += F("\"ch\":");        j += getChannel();    j += ',';
    j += F("\"selected\":"); j += DStr::boolean(_selected); j += ',';
    j += F("\"vendor\":\""); j += DStr::jsonEscape(getVendor()); j += F("\",");
    j += F("\"probes\":\""); j += DStr::jsonEscape(getProbesStr()); j += F("\"");
    j += '}';
    return j;
}

void DeautherStation::print(int id) const {
    if (id >= 0) { Serial.print(DStr::right(3, String(id))); Serial.print(' '); }
    Serial.print(_selected ? F("[*] ") : F("[ ] "));
    Serial.print(getMACStr());
    Serial.print(F("  ap:"));
    Serial.print(DStr::left(18, getAPBSSID()));
    Serial.print(F("  rssi:"));
    Serial.print(DStr::right(4, String(_rssi)));
    Serial.print(F("  pkts:"));
    Serial.print(DStr::right(6, String(_pkts)));
    Serial.print(F("  "));
    Serial.println(DStr::left(20, getVendor()));
}
