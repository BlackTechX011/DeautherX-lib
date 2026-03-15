/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#pragma once

#include <Arduino.h>
#include "DeautherAP.h"

/**
 * DeautherAPList — Linked list container for Access Points.
 *
 * Provides O(n) search by BSSID, select/deselect, sort, iteration,
 * and channel-sorted ordering needed by the attack engine.
 */
class DeautherAPList {
public:
    DeautherAPList(int maxSize = 0); // 0 = unlimited
    ~DeautherAPList();

    // ── Mutation ─────────────────────────────────────────────────────────────
    /**
     * Add an AP (or update RSSI if BSSID already present).
     * @return true if a new AP was inserted, false if updated.
     */
    bool push(const char* ssid, const uint8_t* bssid,
              int8_t rssi, uint8_t enc, uint8_t ch, bool hidden);

    void clear();

    // ── Query ────────────────────────────────────────────────────────────────
    DeautherAP* findByBSSID(const uint8_t* bssid) const;
    DeautherAP* get(int i) const;          // 0-indexed
    int         size() const;
    bool        isFull() const;

    // ── Iterator ─────────────────────────────────────────────────────────────
    void        begin();
    DeautherAP* iterate();
    bool        available() const;

    // ── Selection ────────────────────────────────────────────────────────────
    void selectAll();
    void deselectAll();
    void selectByBSSID(const uint8_t* bssid);
    void selectBySSID(const char* ssid);
    int  countSelected() const;

    // ── Sort ─────────────────────────────────────────────────────────────────
    /** Sort APs by channel number (needed by attack engine for efficient hopping) */
    void sortByChannel();

    // ── Serialization ────────────────────────────────────────────────────────
    String toJSON() const;
    void   print()  const;

private:
    DeautherAP* _head     = nullptr;
    DeautherAP* _tail     = nullptr;
    DeautherAP* _iter     = nullptr;
    int         _size     = 0;
    int         _maxSize;
};
