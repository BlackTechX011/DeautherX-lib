/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#pragma once

#include <Arduino.h>

// ─── Encryption constants ──────────────────────────────────────────────────────
#define DENC_OPEN  0
#define DENC_WEP   1
#define DENC_WPA   2
#define DENC_WPA2  3
#define DENC_WPA_WPA2 4
#define DENC_WPA3  5

/**
 * DeautherAP — A single WiFi Access Point found during a scan.
 *
 * Carries all fields available from an ESP8266 AP scan plus extras
 * needed for attack targeting (selected flag, channel, vendor lookup).
 */
class DeautherAP {
public:
    // ── Construction ────────────────────────────────────────────────────────
    DeautherAP(const char* ssid, const uint8_t* bssid,
               int8_t rssi, uint8_t enc, uint8_t ch, bool hidden);
    ~DeautherAP();

    // ── Getters ─────────────────────────────────────────────────────────────
    const char*    getSSID()    const;
    const uint8_t* getBSSID()   const;
    String         getSSIDStr() const;
    String         getBSSIDStr()const;
    int8_t         getRSSI()    const;
    uint8_t        getEnc()     const;
    String         getEncStr()  const;
    uint8_t        getChannel() const;
    bool           isHidden()   const;
    bool           isSelected() const;
    String         getVendor()  const;

    // ── Selection ───────────────────────────────────────────────────────────
    void select();
    void deselect();
    void toggleSelected();

    // ── Linked list ─────────────────────────────────────────────────────────
    DeautherAP* next = nullptr;

    // ── Serialization ───────────────────────────────────────────────────────
    String toJSON(int id = -1) const;
    void   print(int id = -1)  const;

private:
    char*   _ssid    = nullptr;
    uint8_t _bssid[6];
    int8_t  _rssi    = 0;
    uint8_t _enc     = DENC_OPEN;
    uint8_t _ch      = 1;
    bool    _hidden  = false;
    bool    _selected = false;
};
