/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#pragma once

#include <Arduino.h>
#include "DeautherAttackConfig.h"
#include "../scan/DeautherScanner.h"

/**
 * DeautherAttack — Unified attack engine for deauth, beacon, and probe attacks.
 *
 * Manages all 3 attack types simultaneously. Just call update() from loop().
 *
 * Features drawn from all 3 versions:
 *   - Deauth: targets AP/Station/Both, reason code, deauth+disassoc both directions (DeautherX)
 *   - Beacon: per-SSID rate, WPA2/Open, custom BSSID, 100ms/1s interval, random TX (v2)
 *   - Probe:  flood per SSID, random source MAC (v3)
 *   - All:    timeout, max packet limit, packet-rate counters, JSON status
 */
class DeautherAttack {
public:
    // ── Start attacks ─────────────────────────────────────────────────────────
    void startDeauth (const DeauthConfig&  cfg);
    void startBeacon (const BeaconConfig&  cfg);
    void startProbe  (const ProbeConfig&   cfg);

    /** Convenience: start all three at once (pass nullptr to skip one) */
    void start(const DeauthConfig* d = nullptr,
               const BeaconConfig* b = nullptr,
               const ProbeConfig*  p = nullptr);

    // ── Stop attacks ──────────────────────────────────────────────────────────
    void stopDeauth();
    void stopBeacon();
    void stopProbe();
    void stop();

    // ── Main loop ─────────────────────────────────────────────────────────────
    /** MUST be called every iteration of loop() */
    void update();

    // ── Status ────────────────────────────────────────────────────────────────
    bool isRunning()       const;
    bool deauthRunning()   const;
    bool beaconRunning()   const;
    bool probeRunning()    const;

    uint32_t getPacketRate()   const;
    uint32_t getDeauthPkts()   const;
    uint32_t getBeaconPkts()   const;
    uint32_t getProbePkts()    const;
    unsigned long getRuntime() const;  ///< ms since last start()
    String   getStatusJSON()   const;
    void     printStatus()     const;

    // ── Low-level single-shot frame senders ───────────────────────────────────
    /** Send one deauth frame right now */
    bool sendDeauth (const uint8_t* ap, const uint8_t* sta, uint8_t reason = 1, uint8_t ch = 1);
    /** Send one beacon frame right now */
    bool sendBeacon (const uint8_t* bssid, const char* ssid, uint8_t ch, bool wpa2 = false);
    /** Send one probe request frame right now */
    bool sendProbe  (const uint8_t* src, const char* ssid, uint8_t ch);
    /** Send any raw 802.11 frame right now */
    bool sendRaw    (const uint8_t* buf, uint16_t len, uint8_t ch);

    // ── Callbacks ─────────────────────────────────────────────────────────────
    using PacketSentCb = void (*)(uint8_t type, uint32_t total);
    void onPacketSent(PacketSentCb cb);

private:
    // State machine for each attack type
    struct AttackState {
        bool          active        = false;
        uint32_t      pkt_counter   = 0;  // sent this second
        uint32_t      total_pkts    = 0;  // all time
        unsigned long last_pkt_time = 0;
        uint8_t       target_idx    = 0;
        uint8_t       ssid_idx      = 0;
    };

    AttackState _deauth_state;
    AttackState _beacon_state;
    AttackState _probe_state;

    DeauthConfig  _deauth_cfg;
    BeaconConfig  _beacon_cfg;
    ProbeConfig   _probe_cfg;

    unsigned long _attack_start   = 0;
    unsigned long _last_sec_tick  = 0;
    uint32_t      _pkt_rate       = 0;
    uint32_t      _tmp_pkt_rate   = 0;

    PacketSentCb  _pkt_cb         = nullptr;

    // Internal per-type update methods
    void _updateDeauth();
    void _updateBeacon();
    void _updateProbe();
    void _tickPerSecond();
};
