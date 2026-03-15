/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#include "DeautherAP.h"
#include "../utils/DeautherMac.h"
#include "../utils/DeautherStrHelper.h"
#include "../utils/DeautherVendor.h"

DeautherAP::DeautherAP(const char* ssid, const uint8_t* bssid,
                       int8_t rssi, uint8_t enc, uint8_t ch, bool hidden)
    : _rssi(rssi), _enc(enc), _ch(ch), _hidden(hidden)
{
    if (ssid) {
        uint8_t len = strlen(ssid);
        if (len > 32) len = 32;
        _ssid = new char[len + 1];
        memcpy(_ssid, ssid, len);
        _ssid[len] = '\0';
    } else {
        _ssid = new char[1];
        _ssid[0] = '\0';
    }
    if (bssid) memcpy(_bssid, bssid, 6);
    else       memset(_bssid, 0, 6);
}

DeautherAP::~DeautherAP() {
    delete[] _ssid;
}

const char*    DeautherAP::getSSID()    const { return _ssid ? _ssid : ""; }
const uint8_t* DeautherAP::getBSSID()   const { return _bssid; }
String         DeautherAP::getSSIDStr() const { return String(_ssid ? _ssid : ""); }
String         DeautherAP::getBSSIDStr()const { return DMac::toStr(_bssid); }
int8_t         DeautherAP::getRSSI()    const { return _rssi; }
uint8_t        DeautherAP::getEnc()     const { return _enc; }
uint8_t        DeautherAP::getChannel() const { return _ch; }
bool           DeautherAP::isHidden()   const { return _hidden; }
bool           DeautherAP::isSelected() const { return _selected; }

String DeautherAP::getEncStr() const {
    switch (_enc) {
        case DENC_OPEN:     return F("Open");
        case DENC_WEP:      return F("WEP");
        case DENC_WPA:      return F("WPA");
        case DENC_WPA2:     return F("WPA2");
        case DENC_WPA_WPA2: return F("WPA/WPA2");
        case DENC_WPA3:     return F("WPA3");
        default:            return F("Unknown");
    }
}

String DeautherAP::getVendor() const {
    return DVendor::getName(_bssid);
}

void DeautherAP::select()         { _selected = true;  }
void DeautherAP::deselect()       { _selected = false; }
void DeautherAP::toggleSelected() { _selected = !_selected; }

String DeautherAP::toJSON(int id) const {
    String j = F("{");
    if (id >= 0) { j += F("\"id\":"); j += id; j += ','; }
    j += F("\"ssid\":\"");     j += DStr::jsonEscape(getSSIDStr()); j += F("\",");
    j += F("\"bssid\":\"");    j += getBSSIDStr(); j += F("\",");
    j += F("\"rssi\":");       j += _rssi; j += ',';
    j += F("\"enc\":\"");      j += getEncStr(); j += F("\",");
    j += F("\"ch\":");         j += _ch; j += ',';
    j += F("\"hidden\":");     j += DStr::boolean(_hidden); j += ',';
    j += F("\"selected\":");   j += DStr::boolean(_selected); j += ',';
    j += F("\"vendor\":\"");   j += DStr::jsonEscape(getVendor()); j += F("\"");
    j += '}';
    return j;
}

void DeautherAP::print(int id) const {
    if (id >= 0) { Serial.print(DStr::right(3, String(id))); Serial.print(' '); }
    Serial.print(_selected ? F("[*] ") : F("[ ] "));
    Serial.print(DStr::left(32, getSSIDStr()));
    Serial.print(' ');
    Serial.print(getBSSIDStr());
    Serial.print(F("  ch:"));
    Serial.print(DStr::right(2, String(_ch)));
    Serial.print(F("  rssi:"));
    Serial.print(DStr::right(4, String(_rssi)));
    Serial.print(F("  "));
    Serial.print(DStr::left(10, getEncStr()));
    Serial.print(F("  "));
    Serial.println(DStr::left(20, getVendor()));
}
