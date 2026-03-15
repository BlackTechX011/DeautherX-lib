/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#pragma once

#include <Arduino.h>

namespace DStr {

    /** Pad/truncate string to exactly `width` chars (left-aligned) */
    String left(int width, const String& s);

    /** Pad/truncate string to exactly `width` chars (right-aligned) */
    String right(int width, const String& s);

    /** Center-pad to `width` */
    String center(int width, const String& s);

    /** Format MAC bytes → "AA:BB:CC:DD:EE:FF" */
    String mac(const uint8_t* b, uint8_t len = 6);

    /** Escape special JSON characters in a string */
    String jsonEscape(const String& s);

    /** Format milliseconds → human-readable "1h 23m 45s" */
    String duration(unsigned long ms);

    /** Format a channel bitmask → "1,2,6,11" */
    String channels(uint16_t mask);

    /** Bool → "true"/"false" */
    String boolean(bool v);

    /** Integer → fixed-width decimal string (right-aligned) */
    String fixed(long v, int width);

} // namespace DStr
