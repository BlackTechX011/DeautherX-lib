/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#include "DeautherScanner.h"
#include "../radio/DeautherRadio.h"
#include "../utils/DeautherChannel.h"
#include "../utils/DeautherMac.h"

extern "C" {
#include "user_interface.h"
}

DeautherScanner* DeautherScanner::_instance = nullptr;

// ─── Callback Registration ────────────────────────────────────────────────────

void DeautherScanner::onAPFound     (APFoundCb      cb) { _ap_cb     = cb; }
void DeautherScanner::onStationFound(StationFoundCb cb) { _st_cb     = cb; }
void DeautherScanner::onRSSIUpdate  (RSSIUpdateCb   cb) { _rssi_cb   = cb; }
void DeautherScanner::onDeauthSeen  (DeauthSeenCb   cb) { _deauth_cb = cb; }
void DeautherScanner::onRawFrame    (RawFrameCb     cb) { _raw_cb    = cb; }

// ─── AP Scan ──────────────────────────────────────────────────────────────────

void DeautherScanner::startAP(ScanConfig cfg) {
    if (!cfg.retain) _aps.clear();
    _ap_active = true;

    struct scan_config sc;
    memset(&sc, 0, sizeof(sc));

    if ((cfg.channels & 0x3FFF) != 0x3FFF) {
        // Pick first set channel
        for (uint8_t i = 0; i < 14; i++) {
            if (cfg.channels & (1 << i)) { sc.channel = i + 1; break; }
        }
    }

    _instance = this;
    wifi_set_opmode(STATION_MODE);
    wifi_station_scan(&sc, DeautherScanner::_apScanDoneCallback);
}

void DeautherScanner::_apScanDoneCallback(void* arg, STATUS status) {
    if (!_instance) return;
    _instance->_ap_active = false;
    if (status != OK || !arg) return;

    bss_info* bss = (bss_info*)arg;
    while (bss) {
        bool isNew = _instance->_aps.push(
            (const char*)bss->ssid,
            bss->bssid,
            bss->rssi,
            bss->authmode,
            bss->channel,
            bss->is_hidden
        );
        if (isNew && _instance->_ap_cb) {
            DeautherAP* ap = _instance->_aps.findByBSSID(bss->bssid);
            if (ap) _instance->_ap_cb(*ap);
        }
        bss = STAILQ_NEXT(bss, next);
    }
}

void DeautherScanner::stopAP()  { _ap_active = false; }
bool DeautherScanner::apScanActive() const { return _ap_active; }

// ─── Station Scan ─────────────────────────────────────────────────────────────

void DeautherScanner::startST(ScanConfig cfg) {
    _st_cfg = cfg;
    if (!cfg.retain) _stations.clear();
    _st_active = true;
    _scan_start = millis();
    _ch_last_switch = millis();
    _instance = this;
    DRadio::enablePromiscuous(_promiscuousCb);
    DChannel::resetIterator();
}

void DeautherScanner::stopST() {
    if (!_auth_active && !_rssi_active && !_sniffer_active)
        DRadio::disablePromiscuous();
    _st_active = false;
}

bool DeautherScanner::stScanActive() const { return _st_active; }

// ─── Auth Scan ────────────────────────────────────────────────────────────────

void DeautherScanner::startAuth(ScanConfig cfg) {
    _auth_cfg = cfg;
    _auth_active = true;
    _scan_start = millis();
    _ch_last_switch = millis();
    _instance = this;
    DRadio::enablePromiscuous(_promiscuousCb);
}

void DeautherScanner::stopAuth() {
    if (!_st_active && !_rssi_active && !_sniffer_active)
        DRadio::disablePromiscuous();
    _auth_active = false;
}

bool DeautherScanner::authScanActive() const { return _auth_active; }

// ─── RSSI Scan ────────────────────────────────────────────────────────────────

void DeautherScanner::startRSSI(RSSIScanConfig cfg) {
    _rssi_cfg = cfg;
    _rssi_active = true;
    _scan_start = millis();
    _instance = this;
    DRadio::enablePromiscuous(_promiscuousCb);
}

void DeautherScanner::stopRSSI() {
    if (!_st_active && !_auth_active && !_sniffer_active)
        DRadio::disablePromiscuous();
    _rssi_active = false;
}

bool DeautherScanner::rssiScanActive() const { return _rssi_active; }

// ─── Sniffer ──────────────────────────────────────────────────────────────────

void DeautherScanner::startSniffer(uint16_t channels, uint8_t frameFilter) {
    _sniffer_channels = channels;
    _sniffer_filter   = frameFilter;
    _sniffer_active   = true;
    _deauth_count     = 0;
    _pkt_counter      = 0;
    _scan_start       = millis();
    _ch_last_switch   = millis();
    _instance         = this;
    DRadio::enablePromiscuous(_promiscuousCb);
    DChannel::resetIterator();
}

void DeautherScanner::stopSniffer() {
    if (!_st_active && !_auth_active && !_rssi_active)
        DRadio::disablePromiscuous();
    _sniffer_active = false;
}

bool DeautherScanner::snifferActive() const { return _sniffer_active; }

// ─── Universal ────────────────────────────────────────────────────────────────

void DeautherScanner::stop() {
    _ap_active = _st_active = _auth_active = _rssi_active = _sniffer_active = false;
    DRadio::disablePromiscuous();
}

bool DeautherScanner::active() const {
    return _ap_active || _st_active || _auth_active || _rssi_active || _sniffer_active;
}

// ─── Update loop ──────────────────────────────────────────────────────────────

void DeautherScanner::update() {
    unsigned long now = millis();

    auto anyPromisc = _st_active || _auth_active || _rssi_active || _sniffer_active;

    // Channel hopping
    if (anyPromisc && (now - _ch_last_switch >= _st_cfg.ch_time)) {
        _hopChannel();
        _ch_last_switch = now;
    }

    // Packet rate counter
    if (now - _last_pkt_sec >= 1000) {
        _pkt_rate    = _pkt_counter;
        _pkt_counter = 0;
        _last_pkt_sec = now;
    }

    // Timeout checks
    if (_st_active && _st_cfg.timeout > 0 &&
        (now - _scan_start >= _st_cfg.timeout)) {
        stopST();
    }
    if (_auth_active && _auth_cfg.timeout > 0 &&
        (now - _scan_start >= _auth_cfg.timeout)) {
        stopAuth();
    }
}

void DeautherScanner::_hopChannel() {
    uint16_t mask = _st_active    ? _st_cfg.channels     :
                    _sniffer_active ? _sniffer_channels   :
                    _auth_active  ? _auth_cfg.channels    :
                    0x3FFF;
    DChannel::next(mask);
}

// ─── Promiscuous Frame Handler ─────────────────────────────────────────────────

void DeautherScanner::_promiscuousCb(uint8_t* buf, uint16_t len) {
    if (_instance) _instance->_handleFrame(buf, len, 0);
}

// IEEE 802.11 frame layout offsets (after the sdk-added rssi byte)
#define DFRAME_TYPE(b)    ((b)[12])  // offset within ESP sniffer buffer
#define DFRAME_SUBTYPE(b) (((b)[12] >> 4) & 0x0F)
#define DFRAME_SRC(b)     (&(b)[16])
#define DFRAME_BSSID(b)   (&(b)[22])
#define DFRAME_DST(b)     (&(b)[10])

void DeautherScanner::_handleFrame(uint8_t* buf, uint16_t len, int8_t rssi) {
    if (!buf || len < 12) return;

    // ESP8266 SDK prepends RxControl (12 bytes) then the frame
    uint8_t* frame = buf + 12;
    uint16_t flen  = len - 12;
    if (flen < 24) return;

    // Extract RSSI from RxControl.rssi (byte 0) — signed
    int8_t frame_rssi = (int8_t)buf[0];

    uint8_t type    = frame[0] & 0x0F;
    uint8_t subtype = (frame[0] >> 4) & 0x0F;

    // Raw frame callback (sniffer mode)
    if (_sniffer_active && _raw_cb) {
        _raw_cb(frame, flen, frame_rssi);
    }
    _pkt_counter++;

    // Management frames
    if (type == 0x00) {
        uint8_t* src    = &frame[10];
        uint8_t* dst    = &frame[4];
        uint8_t* bssid  = &frame[16];

        // Deauth (0xC0) or Disassoc (0xA0)
        if ((frame[0] == 0xC0 || frame[0] == 0xA0) && flen >= 26) {
            _deauth_count++;
            if (_deauth_cb) _deauth_cb(src, dst, frame[24]);
        }

        // Station scan — data + management frames
        if (_st_active) {
            bool newST = _stations.push(src, frame_rssi, &_aps, bssid);
            if (newST && _st_cb) {
                DeautherStation* st = _stations.findByMAC(src);
                if (st) _st_cb(*st);
            }
        }

        // Probe Request (0x40) — extract SSID
        if (frame[0] == 0x40 && _st_active && flen > 25) {
            uint8_t ssid_len = frame[25];
            if (ssid_len > 0 && ssid_len <= 32 && flen >= 26u + ssid_len) {
                DeautherStation* st = _stations.findByMAC(src);
                if (st) st->addProbe((const char*)&frame[26], ssid_len);
            }
        }
    }
}

// ─── Results ─────────────────────────────────────────────────────────────────

DeautherAPList&      DeautherScanner::getAPs()      { return _aps; }
DeautherStationList& DeautherScanner::getStations() { return _stations; }
void                 DeautherScanner::clearAPs()     { _aps.clear(); }
void                 DeautherScanner::clearStations(){ _stations.clear(); }
void                 DeautherScanner::clearAll()     { _aps.clear(); _stations.clear(); }

// ─── Stats ───────────────────────────────────────────────────────────────────

uint32_t DeautherScanner::getPacketRate()   const { return _pkt_rate; }
uint32_t DeautherScanner::getDeauthCount()  const { return _deauth_count; }
uint8_t  DeautherScanner::getCurrentChannel() const { return DChannel::current(); }

uint8_t DeautherScanner::getScanPercent() const {
    unsigned long timeout = _st_active ? _st_cfg.timeout : _auth_cfg.timeout;
    if (timeout == 0) return 0;
    unsigned long elapsed = millis() - _scan_start;
    return (uint8_t)min(100UL, (elapsed * 100) / timeout);
}

String DeautherScanner::getStatusJSON() const {
    String j = F("{\"aps\":");
    j += _aps.size();
    j += F(",\"stations\":"); j += _stations.size();
    j += F(",\"pkt_rate\":"); j += _pkt_rate;
    j += F(",\"deauths\":"); j += _deauth_count;
    j += F(",\"channel\":"); j += DChannel::current();
    j += F(",\"percent\":"); j += getScanPercent();
    j += F(",\"active\":"); j += (active() ? F("true") : F("false"));
    j += '}';
    return j;
}

// ─── Selection Helpers ────────────────────────────────────────────────────────

void DeautherScanner::selectAllAPs()                              { _aps.selectAll(); }
void DeautherScanner::deselectAllAPs()                            { _aps.deselectAll(); }
void DeautherScanner::selectAllStations()                         { _stations.selectAll(); }
void DeautherScanner::deselectAllStations()                       { _stations.deselectAll(); }
void DeautherScanner::selectAPsBySSID(const char* ssid)           { _aps.selectBySSID(ssid); }
void DeautherScanner::selectAPByBSSID(const uint8_t* bssid)       { _aps.selectByBSSID(bssid); }
void DeautherScanner::selectStationsByBSSID(const uint8_t* bssid) { _stations.selectByBSSID(bssid); }

void DeautherScanner::printAPs()      const { _aps.print(); }
void DeautherScanner::printStations() const { _stations.print(); }
