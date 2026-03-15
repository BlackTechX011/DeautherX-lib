/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#include "DeautherStrHelper.h"

namespace DStr {

String left(int width, const String& s) {
    String r = s;
    if (r.length() > (unsigned)width) { r = r.substring(0, width); }
    while ((int)r.length() < width) r += ' ';
    return r;
}

String right(int width, const String& s) {
    String r = s;
    if (r.length() > (unsigned)width) { r = r.substring(0, width); }
    while ((int)r.length() < width) r = ' ' + r;
    return r;
}

String center(int width, const String& s) {
    int pad = width - (int)s.length();
    if (pad <= 0) return s.substring(0, width);
    int lpad = pad / 2;
    int rpad = pad - lpad;
    String r;
    for (int i = 0; i < lpad; i++) r += ' ';
    r += s;
    for (int i = 0; i < rpad; i++) r += ' ';
    return r;
}

String mac(const uint8_t* b, uint8_t len) {
    if (!b) return String(F("00:00:00:00:00:00"));
    String r;
    for (uint8_t i = 0; i < len; i++) {
        if (i) r += ':';
        if (b[i] < 0x10) r += '0';
        r += String(b[i], HEX);
    }
    r.toUpperCase();
    return r;
}

String jsonEscape(const String& s) {
    String r;
    for (unsigned i = 0; i < s.length(); i++) {
        char c = s[i];
        switch (c) {
            case '"':  r += F("\\\""); break;
            case '\\': r += F("\\\\"); break;
            case '\n': r += F("\\n");  break;
            case '\r': r += F("\\r");  break;
            case '\t': r += F("\\t");  break;
            default:   r += c;         break;
        }
    }
    return r;
}

String duration(unsigned long ms) {
    unsigned long s   = ms / 1000;
    unsigned long m   = s / 60;  s %= 60;
    unsigned long h   = m / 60;  m %= 60;
    String r;
    if (h) { r += String(h); r += F("h "); }
    if (h || m) { r += String(m); r += F("m "); }
    r += String(s); r += 's';
    return r;
}

String channels(uint16_t mask) {
    String r;
    bool first = true;
    for (uint8_t i = 0; i < 14; i++) {
        if (mask & (1 << i)) {
            if (!first) r += ',';
            r += String(i + 1);
            first = false;
        }
    }
    return r.isEmpty() ? String(F("none")) : r;
}

String boolean(bool v) {
    return v ? F("true") : F("false");
}

String fixed(long v, int width) {
    return right(width, String(v));
}

} // namespace DStr
