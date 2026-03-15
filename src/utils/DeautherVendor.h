/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#pragma once

#include <Arduino.h>

/**
 * DVendor — OUI vendor lookup using the embedded vendor_list (from v3).
 *
 * The vendor database is compiled into oui.h / vendor_list.h from the
 * original projects. This module provides a clean API over it.
 */
namespace DVendor {

    /**
     * Look up the vendor/manufacturer name for a MAC address using the
     * first 3 bytes (OUI prefix) against the embedded database.
     *
     * @param mac  6-byte MAC address
     * @return     Vendor name string, or empty string if not found
     */
    String getName(const uint8_t* mac);

    /**
     * Generate a random MAC address where the first 3 bytes are a valid,
     * randomly-chosen OUI prefix from the database.
     *
     * @param mac  Output 6-byte buffer
     */
    void randomMac(uint8_t* mac);

    /**
     * Search vendors by name substring and call callback for each match.
     * Useful for finding all MACs for a given vendor like "Apple" or "Samsung".
     *
     * @param name      Search string (case-insensitive substring)
     * @param cb        Callback: (prefix[3], vendor_name)
     */
    using SearchCb = void (*)(const uint8_t* prefix, const char* name);
    void search(const char* name, SearchCb cb);

} // namespace DVendor
