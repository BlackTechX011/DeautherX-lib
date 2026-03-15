/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#pragma once

#include <Arduino.h>
#include "DeautherAP.h"

/**
 * DeautherStation — A client device observed during a station scan.
 *
 * Tracks: MAC, associated AP, RSSI, packet count, probe requests, vendor.
 */
class DeautherStation {
public:
    // ── Construction ────────────────────────────────────────────────────────
    DeautherStation(const uint8_t* mac, DeautherAP* ap = nullptr);
    ~DeautherStation();

    // ── Getters ─────────────────────────────────────────────────────────────
    const uint8_t* getMAC()       const;
    String         getMACStr()    const;
    DeautherAP*    getAP()        const;
    String         getAPSSID()    const;
    String         getAPBSSID()   const;
    int8_t         getRSSI()      const;
    uint32_t       getPackets()   const;
    uint8_t        getChannel()   const;
    bool           isSelected()   const;
    String         getVendor()    const;

    /** Return comma-separated list of SSIDs this station has probed for */
    String         getProbesStr() const;

    // ── Mutators ────────────────────────────────────────────────────────────
    void setAP(DeautherAP* ap);
    void addProbe(const char* ssid, uint8_t len);
    void recordPacket(int8_t rssi);

    // ── Selection ───────────────────────────────────────────────────────────
    void select();
    void deselect();
    void toggleSelected();

    // ── Linked list ─────────────────────────────────────────────────────────
    DeautherStation* next = nullptr;

    // ── Serialization ───────────────────────────────────────────────────────
    String toJSON(int id = -1) const;
    void   print(int id = -1)  const;

private:
    uint8_t     _mac[6];
    DeautherAP* _ap      = nullptr;
    int8_t      _rssi    = -127;
    uint32_t    _pkts    = 0;
    bool        _selected = false;

    // Probe list (up to 8 entries, sorted, deduplicated)
    static const uint8_t MAX_PROBES = 8;
    char*   _probes[MAX_PROBES];
    uint8_t _probe_count = 0;
};
