/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#include "DeautherVendor.h"

// Include the large OUI vendor table from v3 (reused as-is)
// The vendor_list.h file contains: vendor_t vendor_list[] = {...};
// where vendor_t = { uint8_t prefix[3]; const char* name; }
#include "../../v3/esp8266_deauther/vendor_list.h"

namespace DVendor {

// ===== Private ===== //
static int bin_search(const uint8_t* bytes, int low_end, int up_end) {
    uint8_t list_bytes[3];
    int     res;
    int     mid = (low_end + up_end) / 2;

    // Check bounds
    list_bytes[0] = pgm_read_byte_near(vendor_macs + up_end*5);
    list_bytes[1] = pgm_read_byte_near(vendor_macs + up_end*5 + 1);
    list_bytes[2] = pgm_read_byte_near(vendor_macs + up_end*5 + 2);
    res           = memcmp(bytes, list_bytes, 3);
    if (res > 0) return -1;
    if (res == 0) return up_end;

    list_bytes[0] = pgm_read_byte_near(vendor_macs + low_end*5);
    list_bytes[1] = pgm_read_byte_near(vendor_macs + low_end*5 + 1);
    list_bytes[2] = pgm_read_byte_near(vendor_macs + low_end*5 + 2);
    res           = memcmp(bytes, list_bytes, 3);
    if (res < 0) return -1;
    if (res == 0) return low_end;

    while (low_end <= up_end) {
        list_bytes[0] = pgm_read_byte_near(vendor_macs + mid*5);
        list_bytes[1] = pgm_read_byte_near(vendor_macs + mid*5 + 1);
        list_bytes[2] = pgm_read_byte_near(vendor_macs + mid*5 + 2);

        res = memcmp(bytes, list_bytes, 3);

        if (res == 0) {
            return mid;
        } else if (res < 0) {
            up_end = mid - 1;
            mid    = (low_end + up_end) / 2;
        } else if (res > 0) {
            low_end = mid + 1;
            mid     = (low_end + up_end) / 2;
        }
    }
    return -1;
}

String getName(const uint8_t* mac) {
    if (!mac) return String();

    int pos_mac = bin_search(mac, 0, (sizeof(vendor_macs) / 5) - 1);
    if (pos_mac < 0) return String();

    int pos_name = pgm_read_byte_near(vendor_macs + pos_mac*5 + 3) |
                   (pgm_read_byte_near(vendor_macs + pos_mac*5 + 4) << 8);
    
    String name;
    for (int i = 0; i < 8; ++i) {
        char tmp = (char)pgm_read_byte_near(vendor_names + pos_name*8 + i);
        if (tmp != '\0') name += tmp;
    }
    return name;
}

void randomMac(uint8_t* mac) {
    if (!mac) return;
    uint32_t count = sizeof(vendor_macs) / 5;
    if (count == 0) {
        for (uint8_t i = 0; i < 6; i++) mac[i] = (uint8_t)random(256);
        mac[0] = (mac[0] & 0xFE) | 0x02;
        return;
    }
    
    uint32_t num = (uint32_t)random(count);
    for (uint8_t i = 0; i < 3; ++i) {
        mac[i] = pgm_read_byte_near(vendor_macs + num * 5 + i);
    }
    for (uint8_t i = 3; i < 6; ++i) {
        mac[i] = random(256);
    }
}

void search(const char* name, SearchCb cb) {
    if (!name || !cb) return;
    
    String searchName = name;
    searchName.trim();
    searchName = searchName.substring(0, 8);
    searchName.toLowerCase();
    if (searchName.length() < 2) return;

    int len = sizeof(vendor_macs);
    uint8_t prefix[6] = {0};
    char vendor[9];
    vendor[8] = '\0';

    for (int i = 0; i < len; i += 5) {
        int pos_name = pgm_read_byte_near(vendor_macs + i + 3) | 
                       (pgm_read_byte_near(vendor_macs + i + 4) << 8);

        for (int j = 0; j < 8; ++j) {
            vendor[j] = (char)pgm_read_byte_near(vendor_names + pos_name*8 + j);
        }

        String vendorStr = vendor;
        vendorStr.toLowerCase();

        if (vendorStr.indexOf(searchName) >= 0) {
            for (int j = 0; j < 3; ++j) {
                prefix[j] = pgm_read_byte_near(vendor_macs + i + j);
            }
            cb(prefix, vendor);
        }
    }
}

} // namespace DVendor
