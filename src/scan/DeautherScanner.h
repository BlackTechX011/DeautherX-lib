/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#pragma once

#include <Arduino.h>
#include "../data/DeautherAPList.h"
#include "../data/DeautherStationList.h"

// ─── Scan Config ──────────────────────────────────────────────────────────────

/**
 * ScanConfig — common settings for all scan types.
 */
struct ScanConfig {
    uint16_t      channels    = 0x3FFF;  ///< Channel bitmask (all 14 channels)
    unsigned long timeout     = 15000;   ///< Max scan duration in ms (0 = unlimited)
    unsigned long ch_time     = 284;     ///< Time in ms spent on each channel
    bool          retain      = false;   ///< Keep previous results across restarts
};

/**
 * RSSIScanConfig — extended config for RSSI tracking mode.
 */
struct RSSIScanConfig {
    ScanConfig    base;
    uint8_t**     macs      = nullptr;  ///< Array of MAC pointers to track
    uint8_t       mac_count = 0;        ///< Length of macs array
    unsigned long update_ms = 1000;     ///< How often to log RSSI updates
};

// ─── Sniffer frame type filter ────────────────────────────────────────────────
#define DSNIFFER_DEAUTH    0x01
#define DSNIFFER_BEACON    0x02
#define DSNIFFER_PROBE     0x04
#define DSNIFFER_DATA      0x08
#define DSNIFFER_ALL       0xFF

// ─── DeautherScanner ─────────────────────────────────────────────────────────

/**
 * DeautherScanner — Full scan engine combining all features from v1/v2/v3.
 *
 * Supports 5 scan modes:
 *   1. AP Scan     — active scan using ESP8266 SDK (finds all APs)
 *   2. Station     — promiscuous sniff to discover client devices
 *   3. Auth Scan   — capture auth/association frames (handshake detection)
 *   4. RSSI Scan   — continuously monitor signal strength of specific MACs
 *   5. Sniffer     — raw frame counter per channel (v2 style, deauth detection)
 *
 * Must call update() in your loop() for ST/Auth/RSSI/Sniffer modes.
 * AP scan uses the Arduino SDK callback and does not need loop() driving.
 */
class DeautherScanner {
public:
    // ── Callbacks ────────────────────────────────────────────────────────────
    using APFoundCb      = void (*)(const DeautherAP&);
    using StationFoundCb = void (*)(const DeautherStation&);
    using RSSIUpdateCb   = void (*)(const uint8_t* mac, int8_t rssi);
    using DeauthSeenCb   = void (*)(const uint8_t* apMAC, const uint8_t* staMAC, uint8_t reason);
    using RawFrameCb     = void (*)(const uint8_t* buf, uint16_t len, int8_t rssi);

    void onAPFound     (APFoundCb      cb);
    void onStationFound(StationFoundCb cb);
    void onRSSIUpdate  (RSSIUpdateCb   cb);
    void onDeauthSeen  (DeauthSeenCb   cb);
    void onRawFrame    (RawFrameCb     cb);

    // ── AP Scan ──────────────────────────────────────────────────────────────
    void startAP(ScanConfig cfg = ScanConfig{});
    void stopAP();
    bool apScanActive() const;

    // ── Station Scan ─────────────────────────────────────────────────────────
    void startST(ScanConfig cfg = ScanConfig{});
    void stopST();
    bool stScanActive() const;

    // ── Auth Scan (handshake/association capture) ─────────────────────────────
    void startAuth(ScanConfig cfg = ScanConfig{});
    void stopAuth();
    bool authScanActive() const;

    // ── RSSI Tracking Scan ────────────────────────────────────────────────────
    void startRSSI(RSSIScanConfig cfg = RSSIScanConfig{});
    void stopRSSI();
    bool rssiScanActive() const;

    // ── Raw Sniffer (all frame types, channel hop) ────────────────────────────
    void startSniffer(uint16_t channels = 0x3FFF, uint8_t frameFilter = DSNIFFER_ALL);
    void stopSniffer();
    bool snifferActive() const;

    // ── Universal controls ────────────────────────────────────────────────────
    void stop();           ///< Stop ALL scan modes
    void update();         ///< MUST be called from loop()
    bool active() const;   ///< True if any scan mode is running

    // ── Results ───────────────────────────────────────────────────────────────
    DeautherAPList&      getAPs();
    DeautherStationList& getStations();
    void                 clearAPs();
    void                 clearStations();
    void                 clearAll();

    // ── Stats ─────────────────────────────────────────────────────────────────
    uint32_t getPacketRate()  const;   ///< Packets/second (sniffer mode)
    uint32_t getDeauthCount() const;   ///< Deauth frames seen
    uint8_t  getScanPercent() const;   ///< Progress 0-100 (when timeout set)
    uint8_t  getCurrentChannel() const;
    String   getStatusJSON()  const;

    // ── Selection helpers (shortcuts for attack preparation) ──────────────────
    void selectAllAPs();
    void deselectAllAPs();
    void selectAllStations();
    void deselectAllStations();
    void selectAPsBySSID(const char* ssid);
    void selectAPByBSSID(const uint8_t* bssid);
    void selectStationsByBSSID(const uint8_t* apBSSID);

    // Print scan results to Serial
    void printAPs()      const;
    void printStations() const;

private:
    DeautherAPList      _aps;
    DeautherStationList _stations;

    // Mode flags
    bool _ap_active      = false;
    bool _st_active      = false;
    bool _auth_active    = false;
    bool _rssi_active    = false;
    bool _sniffer_active = false;

    ScanConfig     _st_cfg;
    ScanConfig     _auth_cfg;
    RSSIScanConfig _rssi_cfg;
    uint16_t       _sniffer_channels   = 0x3FFF;
    uint8_t        _sniffer_filter     = DSNIFFER_ALL;

    unsigned long _scan_start     = 0;
    unsigned long _ch_last_switch  = 0;
    unsigned long _last_pkt_sec    = 0;
    uint32_t      _pkt_counter     = 0;
    uint32_t      _pkt_rate        = 0;
    uint32_t      _deauth_count    = 0;

    APFoundCb       _ap_cb      = nullptr;
    StationFoundCb  _st_cb      = nullptr;
    RSSIUpdateCb    _rssi_cb    = nullptr;
    DeauthSeenCb    _deauth_cb  = nullptr;
    RawFrameCb      _raw_cb     = nullptr;

    static DeautherScanner* _instance; // for static callbacks
    static void _promiscuousCb(uint8_t* buf, uint16_t len);
    void _handleFrame(uint8_t* buf, uint16_t len, int8_t rssi);
    void _hopChannel();
    static void _apScanDoneCallback(void* arg, STATUS status);
};
