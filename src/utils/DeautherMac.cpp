/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#include "DeautherMac.h"

namespace DMac {

const uint8_t BROADCAST[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

// ─── Predicates ───────────────────────────────────────────────────────────────

bool isBroadcast(const uint8_t* mac) {
    if (!mac) return false;
    for (uint8_t i = 0; i < 6; i++) if (mac[i] != 0xFF) return false;
    return true;
}

bool isMulticast(const uint8_t* mac) {
    if (!mac) return false;
    return (mac[0] & 0x01) != 0;
}

bool isNull(const uint8_t* mac) {
    if (!mac) return true;
    for (uint8_t i = 0; i < 6; i++) if (mac[i] != 0x00) return false;
    return true;
}

bool equals(const uint8_t* a, const uint8_t* b) {
    if (!a || !b) return false;
    return memcmp(a, b, 6) == 0;
}

// ─── Conversion ───────────────────────────────────────────────────────────────

bool fromStr(const char* str, uint8_t* out) {
    if (!str || !out) return false;
    uint32_t len = strlen(str);
    // Accept "AA:BB:CC:DD:EE:FF" (17) or "AABBCCDDEEFF" (12)
    if (len == 17) {
        for (uint8_t i = 0; i < 6; i++) {
            char buf[3] = { str[i * 3], str[i * 3 + 1], 0 };
            out[i] = (uint8_t)strtoul(buf, nullptr, 16);
        }
        return true;
    } else if (len == 12) {
        for (uint8_t i = 0; i < 6; i++) {
            char buf[3] = { str[i * 2], str[i * 2 + 1], 0 };
            out[i] = (uint8_t)strtoul(buf, nullptr, 16);
        }
        return true;
    }
    return false;
}

String toStr(const uint8_t* mac) {
    if (!mac) return String("00:00:00:00:00:00");
    char buf[18];
    snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(buf);
}

String toStrSafe(const uint8_t* mac, bool hide) {
    if (!mac) return String("00:00:00:00:00:00");
    if (hide) {
        char buf[18];
        snprintf(buf, sizeof(buf), "**:**:**:%02X:%02X:%02X", mac[3], mac[4], mac[5]);
        return String(buf);
    }
    return toStr(mac);
}

// ─── Generation ───────────────────────────────────────────────────────────────

void randomize(uint8_t* mac) {
    if (!mac) return;
    for (uint8_t i = 0; i < 6; i++) mac[i] = (uint8_t)random(256);
    mac[0] &= 0xFE; // clear multicast bit
    mac[0] |= 0x02; // set locally-administered bit
}

void copy(uint8_t* dst, const uint8_t* src) {
    if (dst && src) memcpy(dst, src, 6);
}

} // namespace DMac
