/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#include "DeautherTargetList.h"
#include "../utils/DeautherMac.h"
#include "../utils/DeautherChannel.h"

DeautherTargetList::DeautherTargetList() {
    _targets = new DeautherTarget[MAX_TARGETS];
    _size    = 0;
}

DeautherTargetList::~DeautherTargetList() {
    delete[] _targets;
}

bool DeautherTargetList::addTarget(const uint8_t* apMAC, const uint8_t* stationMAC, uint16_t channelMask) {
    if (_size >= MAX_TARGETS) return false;
    if (apMAC)      DMac::copy(_targets[_size].sender,   apMAC);
    else            memset(_targets[_size].sender, 0, 6);
    if (stationMAC) DMac::copy(_targets[_size].receiver, stationMAC);
    else            DMac::copy(_targets[_size].receiver, DMac::BROADCAST);
    _targets[_size].channels = channelMask;
    _size++;
    return true;
}

bool DeautherTargetList::addAP(const uint8_t* apMAC, uint8_t ch) {
    return addTarget(apMAC, DMac::BROADCAST, DChannel::fromNum(ch));
}

bool DeautherTargetList::addStation(const uint8_t* stationMAC, const uint8_t* apMAC, uint8_t ch) {
    return addTarget(apMAC, stationMAC, DChannel::fromNum(ch));
}

void DeautherTargetList::fromSelectedAPs(DeautherAPList& aps) {
    aps.begin();
    while (aps.available()) {
        DeautherAP* ap = aps.iterate();
        if (ap && ap->isSelected()) {
            addAP(ap->getBSSID(), ap->getChannel());
        }
    }
}

void DeautherTargetList::fromSelectedStations(DeautherStationList& stations) {
    stations.begin();
    while (stations.available()) {
        DeautherStation* st = stations.iterate();
        if (st && st->isSelected()) {
            const uint8_t* apBSSID = st->getAP() ? st->getAP()->getBSSID() : DMac::BROADCAST;
            uint8_t ch = st->getChannel();
            addStation(st->getMAC(), apBSSID, ch);
        }
    }
}

void DeautherTargetList::fromSelection(DeautherAPList& aps, DeautherStationList& stations) {
    fromSelectedAPs(aps);
    fromSelectedStations(stations);
}

void    DeautherTargetList::clear()     { _size = 0; _iter_pos = 0; }
uint8_t DeautherTargetList::size()      const { return _size; }
bool    DeautherTargetList::empty()     const { return _size == 0; }
bool    DeautherTargetList::isFull()    const { return _size >= MAX_TARGETS; }

void                  DeautherTargetList::begin()    { _iter_pos = 0; }
const DeautherTarget* DeautherTargetList::iterate()  {
    const DeautherTarget* r = (_iter_pos < _size) ? &_targets[_iter_pos] : nullptr;
    _iter_pos = (_iter_pos + 1) % (_size ? _size : 1);
    return r;
}
bool DeautherTargetList::available() const { return _size > 0; }
