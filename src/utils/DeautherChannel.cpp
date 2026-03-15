/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#include "DeautherChannel.h"

extern "C" {
#include "user_interface.h"
}

namespace DChannel {

static uint8_t _iterator = 0;
static uint8_t _current_ch = 1;

uint8_t count(uint16_t mask) {
    uint8_t n = 0;
    for (uint8_t i = 0; i < 14; i++) {
        if (mask & (1 << i)) n++;
    }
    return n;
}

uint8_t next(uint16_t mask) {
    if (mask == 0) return _current_ch;
    uint8_t found = 0;
    for (uint8_t attempt = 0; attempt < 14; attempt++) {
        uint8_t bit = (_iterator + attempt) % 14;
        if (mask & (1 << bit)) {
            _iterator = (bit + 1) % 14;
            found = bit + 1; // channels are 1-indexed
            break;
        }
    }
    if (found == 0) found = 1;
    set(found);
    return found;
}

void resetIterator() {
    _iterator = 0;
}

uint16_t fromNum(uint8_t ch) {
    if (ch < 1 || ch > 14) return 0;
    return (uint16_t)(1 << (ch - 1));
}

String toStr(uint16_t mask) {
    String result;
    bool first = true;
    for (uint8_t i = 0; i < 14; i++) {
        if (mask & (1 << i)) {
            if (!first) result += ',';
            result += String(i + 1);
            first = false;
        }
    }
    if (result.isEmpty()) result = F("none");
    return result;
}

void set(uint8_t ch) {
    if (ch < 1 || ch > 14) ch = 1;
    _current_ch = ch;
    wifi_set_channel(ch);
}

uint8_t current() {
    return _current_ch;
}

} // namespace DChannel
