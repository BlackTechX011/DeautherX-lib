/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#pragma once

#include <Arduino.h>
#include "DeautherStation.h"
#include "DeautherAPList.h"

/**
 * DeautherStationList — Linked list of client stations.
 */
class DeautherStationList {
public:
    DeautherStationList(int maxSize = 0);
    ~DeautherStationList();

    /**
     * Add a station, or update packet count/RSSI if MAC already exists.
     * Optionally associate with an AP from the provided APList.
     */
    bool push(const uint8_t* mac, int8_t rssi = -127, DeautherAPList* apList = nullptr, const uint8_t* apBSSID = nullptr);

    void clear();

    DeautherStation* findByMAC(const uint8_t* mac) const;
    DeautherStation* get(int i) const;
    int              size()     const;
    bool             isFull()   const;

    void             begin();
    DeautherStation* iterate();
    bool             available() const;

    void selectAll();
    void deselectAll();
    void selectByBSSID(const uint8_t* apBSSID); // select all stations associated with a given AP
    int  countSelected() const;

    String toJSON() const;
    void   print()  const;

private:
    DeautherStation* _head    = nullptr;
    DeautherStation* _tail    = nullptr;
    DeautherStation* _iter    = nullptr;
    int              _size    = 0;
    int              _maxSize;
};
