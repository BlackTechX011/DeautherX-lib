/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#include "DeautherStationList.h"
#include "../utils/DeautherMac.h"

DeautherStationList::DeautherStationList(int maxSize) : _maxSize(maxSize) {}
DeautherStationList::~DeautherStationList() { clear(); }

bool DeautherStationList::push(const uint8_t* mac, int8_t rssi,
                               DeautherAPList* apList, const uint8_t* apBSSID)
{
    if (!mac) return false;
    // Ignore broadcast/multicast
    if (DMac::isBroadcast(mac) || DMac::isMulticast(mac)) return false;

    DeautherStation* existing = findByMAC(mac);
    if (existing) {
        existing->recordPacket(rssi);
        if (apBSSID && !existing->getAP() && apList) {
            existing->setAP(apList->findByBSSID(apBSSID));
        }
        return false;
    }

    if (_maxSize > 0 && _size >= _maxSize) return false;

    DeautherAP* ap = nullptr;
    if (apList && apBSSID) ap = apList->findByBSSID(apBSSID);

    DeautherStation* st = new DeautherStation(mac, ap);
    st->recordPacket(rssi);

    if (_tail) { _tail->next = st; _tail = st; }
    else       { _head = _tail = st; }
    _size++;
    return true;
}

void DeautherStationList::clear() {
    DeautherStation* cur = _head;
    while (cur) {
        DeautherStation* next = cur->next;
        delete cur;
        cur = next;
    }
    _head = _tail = _iter = nullptr;
    _size = 0;
}

DeautherStation* DeautherStationList::findByMAC(const uint8_t* mac) const {
    DeautherStation* cur = _head;
    while (cur) {
        if (DMac::equals(cur->getMAC(), mac)) return cur;
        cur = cur->next;
    }
    return nullptr;
}

DeautherStation* DeautherStationList::get(int i) const {
    DeautherStation* cur = _head;
    for (int n = 0; cur && n < i; n++) cur = cur->next;
    return cur;
}

int  DeautherStationList::size()   const { return _size; }
bool DeautherStationList::isFull() const { return _maxSize > 0 && _size >= _maxSize; }

void             DeautherStationList::begin()     { _iter = _head; }
DeautherStation* DeautherStationList::iterate()   { DeautherStation* r = _iter; if (_iter) _iter = _iter->next; return r; }
bool             DeautherStationList::available() const { return _iter != nullptr; }

void DeautherStationList::selectAll() {
    DeautherStation* cur = _head;
    while (cur) { cur->select(); cur = cur->next; }
}

void DeautherStationList::deselectAll() {
    DeautherStation* cur = _head;
    while (cur) { cur->deselect(); cur = cur->next; }
}

void DeautherStationList::selectByBSSID(const uint8_t* apBSSID) {
    if (!apBSSID) return;
    DeautherStation* cur = _head;
    while (cur) {
        if (cur->getAP() && DMac::equals(cur->getAP()->getBSSID(), apBSSID)) cur->select();
        cur = cur->next;
    }
}

int DeautherStationList::countSelected() const {
    int n = 0;
    DeautherStation* cur = _head;
    while (cur) { if (cur->isSelected()) n++; cur = cur->next; }
    return n;
}

String DeautherStationList::toJSON() const {
    String j = "[";
    DeautherStation* cur = _head;
    int i = 0;
    while (cur) { if (i) j += ','; j += cur->toJSON(i++); cur = cur->next; }
    j += ']';
    return j;
}

void DeautherStationList::print() const {
    Serial.println(F("  ID  SEL  MAC                AP BSSID            RSSI  PKTS  CH  VENDOR"));
    Serial.println(F("  =============================================================================="));
    DeautherStation* cur = _head;
    int i = 0;
    while (cur) { cur->print(i++); cur = cur->next; }
    Serial.print(F("  Total: ")); Serial.print(_size);
    Serial.print(F("  Selected: ")); Serial.println(countSelected());
}
