/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#pragma once

#include <Arduino.h>

/**
 * DeautherSSIDList — Ordered list of SSIDs + WPA2 flags used by beacon/probe attacks.
 */
class DeautherSSIDList {
public:
    static const uint8_t MAX_LEN = 32;
    static const uint8_t MAX_ENTRIES = 200;

    DeautherSSIDList();
    ~DeautherSSIDList();

    /** Add an SSID. Returns false if full or duplicate. */
    bool add(const char* ssid, bool wpa2 = false);

    /** Remove by index */
    void remove(uint8_t index);

    /** Clear all entries */
    void clear();

    uint8_t     count()             const;
    bool        empty()             const;
    const char* getSSID(uint8_t i)  const;
    bool        getWPA2(uint8_t i)  const;

    /** Iterator — call begin() then iterate() until available() returns false */
    void        begin();
    const char* iterateSSID();
    bool        iterateWPA2();
    bool        available()         const;

    void print() const;

private:
    struct Entry {
        char  ssid[MAX_LEN + 1];
        bool  wpa2;
    };

    Entry*  _entries    = nullptr;
    uint8_t _count      = 0;
    uint8_t _iter_pos   = 0;
    bool    _iter_ready = false;
};
