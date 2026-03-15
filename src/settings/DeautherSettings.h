/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#pragma once

#include <Arduino.h>
#include <EEPROM.h>

// ─── Settings Struct ──────────────────────────────────────────────────────────

/**
 * DSettings — All persistent configuration values stored in EEPROM.
 */
struct DSettings {
    uint32_t magic = 0xDE017A;  ///< Magic number to detect if EEPROM is initialized

    // ── Attack ──
    bool     attack_all_ch         = false; ///< Hop all channels during attack
    bool     random_tx             = false; ///< Randomize TX power per cycle
    uint8_t  deauths_per_target    = 20;    ///< Deauth frames per target per second
    uint8_t  deauth_reason         = 1;     ///< IEEE 802.11 reason code
    bool     beacon_interval_1s    = false; ///< true=1s, false=100ms beacon interval
    uint8_t  probe_frames_per_ssid = 3;     ///< Probe frames per SSID per cycle
    bool     send_deauth           = true;  ///< Send deauth frames
    bool     send_disassoc         = true;  ///< Send disassoc frames

    // ── WiFi ──
    uint8_t  channel       = 1;
    uint8_t  mac_sta[6]    = {0};  ///< Custom STA MAC (all-zero = default)
    uint8_t  mac_ap[6]     = {0};  ///< Custom AP MAC

    // ── Scanner ──
    uint16_t scan_channel_time     = 284;   ///< ms per channel during scan
    uint16_t min_deauth_frames     = 3;     ///< Min deauth frames to flag as attack
    uint32_t scan_timeout          = 15000; ///< Default scan timeout ms

    // ── Evil Twin ──
    char     et_ssid[33]           = "Free WiFi";
    uint8_t  et_channel            = 1;
    bool     et_hidden             = false;

    // Padding for future fields
    uint8_t  _reserved[16]        = {0};
};

// ─── Settings Manager ─────────────────────────────────────────────────────────

/**
 * DSettingsManager — Load/save/reset settings from EEPROM.
 *
 * Call DSettingsManager::load() once in setup().
 * Modify DSettingsManager::get() and call DSettingsManager::save() to persist.
 */
namespace DSettingsManager {

    /** Initialize EEPROM and load settings. If magic number doesn't match, resets to defaults. */
    void load();

    /** Save current settings to EEPROM */
    void save();

    /** Reset to factory defaults and save */
    void reset();

    /** Get the current settings struct (modifiable reference) */
    DSettings& get();

    /** Set entire settings at once */
    void set(const DSettings& s);

    /** Print current settings to Serial */
    void print();

    /** Export settings as JSON */
    String toJSON();

} // namespace DSettingsManager
