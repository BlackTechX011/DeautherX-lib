/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#pragma once

#include <Arduino.h>
#include "../data/DeautherSSIDList.h"
#include "../data/DeautherTargetList.h"

// ─── Deauth Attack Config ─────────────────────────────────────────────────────

/**
 * DeauthConfig — Settings for a deauthentication / disassociation attack.
 *
 * The attack will cycle through all targets in the TargetList,
 * sending deauth and/or disassoc frames at the specified rate.
 */
struct DeauthConfig {
    DeautherTargetList targets;       ///< Who to attack (populated from scan or manually)
    bool               deauth   = true;  ///< Send 802.11 Deauth frames (0xC0)
    bool               disassoc = true;  ///< Send 802.11 Disassoc frames (0xA0)
    uint32_t           pkt_rate = 20;    ///< Packets per second per target
    unsigned long      timeout  = 0;     ///< ms, 0 = run until stopped
    unsigned long      max_pkts = 0;     ///< 0 = unlimited
    uint8_t            reason   = 1;     ///< IEEE 802.11 reason code
    bool               all_ch   = false; ///< Hop all channels (for broadcast deauth on unknown ch)
    bool               random_tx= false; ///< Randomize TX power each cycle
};

// ─── Beacon Flood Config ──────────────────────────────────────────────────────

/**
 * BeaconConfig — Settings for a fake AP beacon flood attack.
 *
 * Sends beacon frames for each SSID in the ssids list,
 * making them visible to all nearby devices as real Wi-Fi networks.
 */
struct BeaconConfig {
    DeautherSSIDList ssids;
    uint8_t          bssid[6]    = { 0xAA, 0xBB, 0xCC, 0x00, 0x00, 0x00 };
    uint8_t          receiver[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    uint16_t         channels    = 0x3FFF; ///< Channels to broadcast on
    uint32_t         pkt_rate    = 10;     ///< Beacons/sec per SSID
    unsigned long    timeout     = 0;
    bool             wpa2        = false;  ///< If true all SSIDs appear WPA2-encrypted
    bool             interval1s  = false;  ///< 1s beacon interval (less aggressive)
    bool             random_mac  = false;  ///< Randomize BSSID per cycle
    bool             random_tx   = false;
};

// ─── Probe Flood Config ───────────────────────────────────────────────────────

/**
 * ProbeConfig — Settings for a probe request flood attack.
 *
 * Spams probe requests for all SSIDs in the list, 
 * cluttering the air with fake device probes.
 */
struct ProbeConfig {
    DeautherSSIDList ssids;
    uint8_t          sender[6]   = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; ///< All-zero = auto-random per packet
    uint8_t          receiver[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    uint16_t         channels    = 0x3FFF;
    uint32_t         pkt_rate    = 10;
    unsigned long    timeout     = 0;
    uint8_t          frames_per_ssid = 3; ///< Number of probe frames per SSID per cycle
    bool             random_mac  = true;  ///< New random source MAC each cycle
};
