/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#pragma once

#include <Arduino.h>

namespace DMac {

    // The broadcast MAC address
    extern const uint8_t BROADCAST[6];

    // ─── Predicates ─────────────────────────────────────────────────────────────

    /** Returns true if mac is FF:FF:FF:FF:FF:FF */
    bool isBroadcast(const uint8_t* mac);

    /** Returns true if the multicast/group bit is set (LSB of first octet) */
    bool isMulticast(const uint8_t* mac);

    /** True if mac is all-zeros */
    bool isNull(const uint8_t* mac);

    /** Byte-wise equality */
    bool equals(const uint8_t* a, const uint8_t* b);

    // ─── Conversion ─────────────────────────────────────────────────────────────

    /**
     * Parse a colon-separated MAC string (e.g. "AA:BB:CC:DD:EE:FF") into a 6-byte array.
     * Returns true on success.
     */
    bool fromStr(const char* str, uint8_t* out);

    /** Format a 6-byte MAC into "AA:BB:CC:DD:EE:FF" */
    String toStr(const uint8_t* mac);

    /** Same but masks the first 3 octets with "**:**:**" when hide_mode is on */
    String toStrSafe(const uint8_t* mac, bool hide = false);

    // ─── Generation ─────────────────────────────────────────────────────────────

    /** Fill mac with a fully random address (locally-administered bit set) */
    void randomize(uint8_t* mac);

    /** Copy src into dst */
    void copy(uint8_t* dst, const uint8_t* src);

} // namespace DMac
