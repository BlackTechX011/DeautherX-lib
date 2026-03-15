/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#pragma once

#include <Arduino.h>

/**
 * Channel bitmask convention (from v3):
 *   Bit 0  = channel 1
 *   Bit 1  = channel 2
 *   ...
 *   Bit 13 = channel 14
 *   0x3FFF = all 14 channels
 */
#define DCHANNEL_ALL    ((uint16_t)0x3FFF)
#define DCHANNEL_COMMON ((uint16_t)0x07FF) // channels 1-11 (most common)

namespace DChannel {

    /** How many channels are set in the bitmask */
    uint8_t  count(uint16_t mask);

    /** Get the next channel from the bitmask (round-robin internal counter) */
    uint8_t  next(uint16_t mask);

    /** Reset the internal round-robin counter */
    void     resetIterator();

    /** Convert a channel number (1-14) to a single-bit bitmask */
    uint16_t fromNum(uint8_t ch);

    /** Convert a bitmask to a human-readable string e.g. "1,2,6,11" */
    String   toStr(uint16_t mask);

    /** Set the ESP8266 radio to a specific channel immediately */
    void     set(uint8_t ch);

    /** Get the currently active radio channel (1-14) */
    uint8_t  current();

} // namespace DChannel
