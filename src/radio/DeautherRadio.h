/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#pragma once

#include <Arduino.h>

extern "C" {
#include "user_interface.h"
}

/**
 * DRadio — Low-level raw packet injection for ESP8266.
 *
 * Wraps `wifi_send_pkt_freedom` (the undocumented Espressif SDK function that
 * allows sending arbitrary 802.11 frames) and the promiscuous mode API.
 */
namespace DRadio {

    using PromiscuousCb = wifi_promiscuous_cb_t;

    /**
     * Send a raw 802.11 frame on the given channel.
     * Automatically switches to `ch` before sending if the radio is not
     * already there (unless force_ch = false).
     *
     * @param ch        Target channel (1-14)
     * @param buf       Frame bytes (complete 802.11 MAC frame, no FCS needed)
     * @param len       Frame length in bytes
     * @param force_ch  If true, always switch channel; if false, skip switch
     * @return true if the frame was handed to the SDK successfully
     */
    bool send(uint8_t ch, const uint8_t* buf, uint16_t len, bool force_ch = true);

    /**
     * Set the transmit power.
     * @param dBm  Output power in dBm (0.0 – 20.5). Values above the HW max are clamped.
     */
    void setTxPower(float dBm);

    /**
     * Set a random TX power in the range [min, max].
     * Useful for beacon/probe attacks that should not look like a single device.
     */
    void setRandomTxPower(float minDBm = 0.0f, float maxDBm = 20.5f);

    /**
     * Enable promiscuous / monitor mode so raw frames can be received.
     * @param cb  Callback function called for every received raw frame.
     */
    void enablePromiscuous(PromiscuousCb cb);

    /** Disable promiscuous mode and restore normal station operation */
    void disablePromiscuous();

    /** True if the radio is currently in promiscuous mode */
    bool isPromiscuous();

    /** Get the last channel the radio was set to */
    uint8_t currentChannel();

} // namespace DRadio
