/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#pragma once

#include <Arduino.h>
#include "DeautherAPList.h"
#include "DeautherStationList.h"

/**
 * DeautherTarget — a single (sender, receiver, channel_mask) tuple for deauth/disassoc.
 *
 *  sender   = AP BSSID  (the "from" field in the 802.11 frame)
 *  receiver = Station MAC (the "to" field; BROADCAST = kick all clients)
 *  channels = bitmask of channels to try (usually just the AP's channel)
 */
struct DeautherTarget {
    uint8_t  sender[6];
    uint8_t  receiver[6];
    uint16_t channels;
};

/**
 * DeautherTargetList — dynamic array of attack targets.
 *
 * Can be populated manually or auto-built from AP/Station selection.
 */
class DeautherTargetList {
public:
    static const uint8_t MAX_TARGETS = 100;

    DeautherTargetList();
    ~DeautherTargetList();

    // ── Manual population ────────────────────────────────────────────────────

    /** Add an explicit (AP, station) pair */
    bool addTarget(const uint8_t* apMAC, const uint8_t* stationMAC, uint16_t channelMask);

    /** Add AP → broadcast (kicks all clients on that AP) */
    bool addAP(const uint8_t* apMAC, uint8_t ch);

    /** Add station → its associated AP */
    bool addStation(const uint8_t* stationMAC, const uint8_t* apMAC, uint8_t ch);

    // ── Auto-build from scan results ─────────────────────────────────────────

    /**
     * Populate from selected APs (receiver = BROADCAST — broadcast deauth
     * kicks every client of the AP simultaneously).
     */
    void fromSelectedAPs(DeautherAPList& aps);

    /**
     * Populate from selected stations (both directions: AP→Station and Station→AP).
     */
    void fromSelectedStations(DeautherStationList& stations);

    /** Populate from both selected APs and selected stations */
    void fromSelection(DeautherAPList& aps, DeautherStationList& stations);

    // ── Management ───────────────────────────────────────────────────────────
    void clear();
    uint8_t size() const;
    bool    empty() const;
    bool    isFull() const;

    // ── Iterator ─────────────────────────────────────────────────────────────
    void                  begin();
    const DeautherTarget* iterate();
    bool                  available() const;

private:
    DeautherTarget* _targets  = nullptr;
    uint8_t         _size     = 0;
    uint8_t         _iter_pos = 0;
};
