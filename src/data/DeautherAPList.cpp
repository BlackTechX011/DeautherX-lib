/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#include "DeautherAPList.h"
#include "../utils/DeautherMac.h"

DeautherAPList::DeautherAPList(int maxSize) : _maxSize(maxSize) {}

DeautherAPList::~DeautherAPList() { clear(); }

bool DeautherAPList::push(const char* ssid, const uint8_t* bssid,
                          int8_t rssi, uint8_t enc, uint8_t ch, bool hidden)
{
    // Update if already exists
    DeautherAP* existing = findByBSSID(bssid);
    if (existing) {
        // Update volatile fields
        return false;
    }
    if (_maxSize > 0 && _size >= _maxSize) return false;

    DeautherAP* ap = new DeautherAP(ssid, bssid, rssi, enc, ch, hidden);
    if (_tail) { _tail->next = ap; _tail = ap; }
    else       { _head = _tail = ap; }
    _size++;
    return true;
}

void DeautherAPList::clear() {
    DeautherAP* cur = _head;
    while (cur) {
        DeautherAP* next = cur->next;
        delete cur;
        cur = next;
    }
    _head = _tail = _iter = nullptr;
    _size = 0;
}

DeautherAP* DeautherAPList::findByBSSID(const uint8_t* bssid) const {
    DeautherAP* cur = _head;
    while (cur) {
        if (DMac::equals(cur->getBSSID(), bssid)) return cur;
        cur = cur->next;
    }
    return nullptr;
}

DeautherAP* DeautherAPList::get(int i) const {
    DeautherAP* cur = _head;
    for (int n = 0; cur && n < i; n++) cur = cur->next;
    return cur;
}

int  DeautherAPList::size()   const { return _size; }
bool DeautherAPList::isFull() const { return _maxSize > 0 && _size >= _maxSize; }

void        DeautherAPList::begin()     { _iter = _head; }
DeautherAP* DeautherAPList::iterate()  { DeautherAP* r = _iter; if (_iter) _iter = _iter->next; return r; }
bool        DeautherAPList::available() const { return _iter != nullptr; }

void DeautherAPList::selectAll() {
    DeautherAP* cur = _head;
    while (cur) { cur->select(); cur = cur->next; }
}

void DeautherAPList::deselectAll() {
    DeautherAP* cur = _head;
    while (cur) { cur->deselect(); cur = cur->next; }
}

void DeautherAPList::selectByBSSID(const uint8_t* bssid) {
    DeautherAP* ap = findByBSSID(bssid);
    if (ap) ap->select();
}

void DeautherAPList::selectBySSID(const char* ssid) {
    if (!ssid) return;
    DeautherAP* cur = _head;
    while (cur) {
        if (strcmp(cur->getSSID(), ssid) == 0) cur->select();
        cur = cur->next;
    }
}

int DeautherAPList::countSelected() const {
    int n = 0;
    DeautherAP* cur = _head;
    while (cur) { if (cur->isSelected()) n++; cur = cur->next; }
    return n;
}

void DeautherAPList::sortByChannel() {
    // Bubble sort by channel (small list, acceptable)
    if (!_head || !_head->next) return;
    bool swapped = true;
    while (swapped) {
        swapped = false;
        DeautherAP* cur = _head;
        DeautherAP* prev = nullptr;
        while (cur && cur->next) {
            if (cur->getChannel() > cur->next->getChannel()) {
                // Swap nodes
                DeautherAP* nxt = cur->next;
                cur->next = nxt->next;
                nxt->next = cur;
                if (prev) prev->next = nxt; else _head = nxt;
                prev = nxt;
                swapped = true;
            } else {
                prev = cur;
                cur = cur->next;
            }
        }
        _tail = cur ? cur : _head;
    }
}

String DeautherAPList::toJSON() const {
    String j = "[";
    DeautherAP* cur = _head;
    int i = 0;
    while (cur) {
        if (i) j += ',';
        j += cur->toJSON(i++);
        cur = cur->next;
    }
    j += ']';
    return j;
}

void DeautherAPList::print() const {
    Serial.println(F("  ID  SEL  SSID                              BSSID              CH  RSSI  ENC        VENDOR"));
    Serial.println(F("  ======================================================================================================"));
    DeautherAP* cur = _head;
    int i = 0;
    while (cur) { cur->print(i++); cur = cur->next; }
    Serial.print(F("  Total: ")); Serial.print(_size);
    Serial.print(F("  Selected: ")); Serial.println(countSelected());
}
