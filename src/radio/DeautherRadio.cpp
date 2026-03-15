#include "DeautherRadio.h"
#include "../utils/DeautherChannel.h"

namespace DRadio {

static bool _promisc = false;

bool send(uint8_t ch, const uint8_t* buf, uint16_t len, bool force_ch) {
    if (!buf || len == 0) return false;
    if (force_ch) DChannel::set(ch);
    // wifi_send_pkt_freedom returns 0 on success
    return wifi_send_pkt_freedom((uint8_t*)buf, len, 0) == 0;
}

void setTxPower(float dBm) {
    if (dBm < 0.0f)   dBm = 0.0f;
    if (dBm > 20.5f)  dBm = 20.5f;
    // SDK expects value in 0.25 dBm units
    system_phy_set_max_tpw((uint8_t)(dBm * 4));
}

void setRandomTxPower(float minDBm, float maxDBm) {
    float range  = maxDBm - minDBm;
    float chosen = minDBm + ((float)random(1000) / 1000.0f) * range;
    setTxPower(chosen);
}

void enablePromiscuous(PromiscuousCb cb) {
    wifi_set_opmode(STATION_MODE);
    wifi_promiscuous_enable(0);
    wifi_set_promiscuous_rx_cb(cb);
    wifi_promiscuous_enable(1);
    _promisc = true;
}

void disablePromiscuous() {
    wifi_promiscuous_enable(0);
    _promisc = false;
}

bool isPromiscuous() {
    return _promisc;
}

uint8_t currentChannel() {
    return DChannel::current();
}

} // namespace DRadio
