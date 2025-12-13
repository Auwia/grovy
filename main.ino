/*
 * Author: Massimo Manganiello
 * version: 1.4 (UI fixes + uptime in page + log UX + OTA menu)
 */

#include <Wire.h>
#include <Adafruit_MCP23X17.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <stdarg.h>

// ===== WIFI CONFIG =====
const char* ssidRouter = "UPC2522560";
const char* passwordRouter = "7FxuryjpTtus";

const char* ssidAP = "Prototype_001";
const char* passwordAP = "PasswordAP";

// true = Access Point, false = Client
bool isAPMode = true;

// ===== WEB SERVER =====
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

// ===== MCP23017 =====
Adafruit_MCP23X17 mcp;
bool mcp_ok = false;

// Stato relè
bool relayState[16] = { LOW };

// ===== LOG BUFFER (Serial + Web) =====
String logBuf;
const size_t LOG_MAX = 8000;

void logAppend(const String& s) {
  Serial.print(s);
  logBuf += s;
  if (logBuf.length() > LOG_MAX) {
    logBuf.remove(0, logBuf.length() - LOG_MAX);
  }
}

void logPrintf(const char* fmt, ...) {
  char tmp[256];
  va_list args;
  va_start(args, fmt);
  vsnprintf(tmp, sizeof(tmp), fmt, args);
  va_end(args);
  logAppend(String(tmp));
}

// ===== helpers =====
String deviceIPString() {
  return isAPMode ? WiFi.softAPIP().toString() : WiFi.localIP().toString();
}

unsigned long timerSeconds() {
  return millis() / 1000UL;
}

String uptimeString() {
  unsigned long s = timerSeconds();
  unsigned long h = s / 3600UL;
  unsigned long m = (s % 3600UL) / 60UL;
  s = s % 60UL;

  char buf[32];
  snprintf(buf, sizeof(buf), "%02luh %02lum %02lus", h, m, s);
  return String(buf);
}

String jsonStatus() {
  String j = "{";
  j += "\"uptime\":\"" + uptimeString() + "\",";
  j += "\"mcp_ok\":" + String(mcp_ok ? "true" : "false") + ",";
  j += "\"relays\":[";
  for (int i = 0; i < 16; i++) {
    j += (relayState[i] ? "1" : "0");
    if (i < 15) j += ",";
  }
  j += "]}";
  return j;
}

void setup() {
  Serial.begin(115200);
  delay(50);

  // WIFI
  if (isAPMode) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssidAP, passwordAP);
    logPrintf("Sono un AP...\n");
    logPrintf("SSID: %s\n", ssidAP);
    logPrintf("IP AP: %s\n", WiFi.softAPIP().toString().c_str());
  } else {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssidRouter, passwordRouter);
    logPrintf("Connessione al router...\n");
    while (WiFi.status() != WL_CONNECTED) {
      delay(250);
      logAppend(".");
      yield();
    }
    logPrintf("\nConnesso, IP: %s\n", WiFi.localIP().toString().c_str());
  }

  // I2C + MCP
  Wire.begin();              // NodeMCU: SDA=D2(GPIO4), SCL=D1(GPIO5)
  mcp_ok = mcp.begin_I2C();  // default 0x20

  if (!mcp_ok) {
    logPrintf("ATTENZIONE: MCP23017 non trovato su I2C (0x20). Continuo senza relè HW.\n");
  } else {
    logPrintf("MCP23017 OK su I2C.\n");
    for (int i = 0; i < 16; i++) {
      mcp.pinMode(i, OUTPUT);
      mcp.digitalWrite(i, LOW);
      relayState[i] = LOW;
    }
  }

  // ===== ROUTES =====

  // HOME
  server.on("/", HTTP_GET, []() {
    String h;
    h.reserve(6500);
    h += "<!doctype html><html><head><meta charset='utf-8'>";
    h += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    h += "<title>Relay Control</title>";
    h += "<style>"
         "body{font-family:Arial;margin:16px}"
         "nav a{margin-right:14px;font-weight:bold}"
         "pre{height:260px;overflow:auto;background:#111;color:#0f0;padding:10px;border-radius:8px}"
         ".warn{color:#b00;font-weight:bold}"
         ".row{display:flex;align-items:center;gap:10px;margin:6px 0}"
         ".led{width:12px;height:12px;border-radius:50%;display:inline-block;border:1px solid #555}"
         ".on{background:#2ecc71}"
         ".off{background:#444}"
         "</style></head><body>";

    h += "<nav>"
         "<a href='/'>Home</a>"
         "<a href='/logs'>Log</a>"
         "<a href='/ota'>OTA</a>"
         "</nav><hr>";

    h += "<h1>Controllo dei Rele</h1>";
    h += "<div>IP: <b>" + deviceIPString() + "</b></div>";
    h += "<div>Uptime: <b id='timer'>--</b></div>";
    h += "<div id='mcp' class='warn' style='display:none'>ATTENZIONE: MCP23017 non trovato!</div>";
    h += "<hr>";

    h += "<h2>Relè</h2>";
    h += "<button id='toggleAll' style='padding:10px 14px;font-weight:bold'>Seleziona tutti</button>";
    h += "<div style='margin-top:10px'></div>";

    for (int i = 0; i < 16; i++) {
      h += "<div class='row'>";
      // checkbox a sinistra
      h += "<input type='checkbox' id='r" + String(i) + "'" + (relayState[i] ? " checked" : "") + ">";
      h += "<label>Rele " + String(i) + "</label>";
      // spia a destra
      h += "<span class='led " + String(relayState[i] ? "on" : "off") + "' id='led" + String(i) + "'></span>";
      h += "</div>";
    }

    h += "<hr><h3>Log (live)</h3>";
    h += "<button id='clearHome'>Svuota log</button>";
    h += "<pre id='log'></pre>";

    // JS
    h += "<script>"
         "function allOn(arr){return arr.every(x=>x===1)}"

         "function render(j){"
         "  document.getElementById('timer').textContent = j.uptime;"
         "  document.getElementById('mcp').style.display = j.mcp_ok ? 'none' : 'block';"
         "  for(let i=0;i<16;i++){"
         "    let cb=document.getElementById('r'+i);"
         "    if(cb) cb.checked = (j.relays[i]===1);"
         "    let led=document.getElementById('led'+i);"
         "    if(led){"
         "      led.classList.remove('on','off');"
         "      led.classList.add(j.relays[i]===1 ? 'on' : 'off');"
         "    }"
         "  }"
         "  let b=document.getElementById('toggleAll');"
         "  if(b){ b.textContent = allOn(j.relays) ? 'Deseleziona tutti' : 'Seleziona tutti'; }"
         "}"

         "async function fetchStatus(){"
         "  try{ let r=await fetch('/status'); render(await r.json()); }catch(e){}"
         "}"

         "async function setRelay(i, state){"
         "  try{ let r=await fetch(`/relay?i=${i}&s=${state?1:0}`); render(await r.json()); }catch(e){}"
         "}"

         "async function setAll(state){"
         "  try{ let r=await fetch(`/relayall?s=${state?1:0}`); render(await r.json()); }catch(e){}"
         "}"

         "async function refreshLog(){"
         "  try{"
         "    let t=await fetch('/logs?raw=1');"
         "    let txt=await t.text();"
         "    let el=document.getElementById('log');"
         "    el.textContent=txt;"
         "    el.scrollTop=999999;"
         "  }catch(e){}"
         "}"

         "window.addEventListener('load', ()=>{"
         "  for(let i=0;i<16;i++){"
         "    let cb=document.getElementById('r'+i);"
         "    if(cb) cb.addEventListener('change', ()=> setRelay(i, cb.checked));"
         "  }"
         "  let b=document.getElementById('toggleAll');"
         "  if(b){"
         "    b.addEventListener('click', async (ev)=>{"
         "      ev.preventDefault();"
         "      let r=await fetch('/status'); let j=await r.json();"
         "      setAll(allOn(j.relays) ? false : true);"
         "    });"
         "  }"
         "  let c=document.getElementById('clearHome');"
         "  if(c){ c.addEventListener('click', async ()=>{ await fetch('/clearlog'); await refreshLog(); }); }"
         "  setInterval(fetchStatus, 1500);"
         "  setInterval(refreshLog, 1000);"
         "  fetchStatus(); refreshLog();"
         "});"
         "</script>";

    h += "</body></html>";
    server.send(200, "text/html; charset=utf-8", h);
  });

  // OTA menu page (link home + link update)
  server.on("/ota", HTTP_GET, []() {
    String h = "<!doctype html><html><head><meta charset='utf-8'>"
               "<meta name='viewport' content='width=device-width, initial-scale=1'>"
               "<title>OTA</title>"
               "<style>body{font-family:Arial;margin:16px} a{font-weight:bold}</style>"
               "</head><body>"
               "<a href='/'>← Home</a> | <a href='/logs'>Log</a><hr>"
               "<h2>Aggiornamento Firmware (OTA)</h2>"
               "<p><a href='/update'>Apri pagina upload firmware</a></p>"
               "</body></html>";
    server.send(200, "text/html; charset=utf-8", h);
  });

  // STATUS JSON
  server.on("/status", HTTP_GET, []() {
    server.send(200, "application/json; charset=utf-8", jsonStatus());
  });

  // Clear log
  server.on("/clearlog", HTTP_GET, []() {
    logBuf = "";
    server.send(200, "text/plain; charset=utf-8", "OK");
  });

  // LOGS (HTML + raw)
  server.on("/logs", HTTP_GET, []() {
    if (server.hasArg("raw")) {
      server.send(200, "text/plain; charset=utf-8", logBuf);
      return;
    }

    String h = "<!doctype html><html><head><meta charset='utf-8'>";
    h += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    h += "<title>Logs</title>";
    h += "<style>body{font-family:Arial;margin:16px}"
         "pre{height:80vh;overflow:auto;background:#111;color:#0f0;padding:10px;border-radius:8px}"
         "a{font-weight:bold} button{margin:10px 0; padding:8px 12px; font-weight:bold}</style></head><body>";
    h += "<a href='/'>← Home</a> | <a href='/ota'>OTA</a>";
    h += "<h2>Debug log</h2>";
    h += "<button id='clear'>Svuota log</button>";
    h += "<pre id='log'></pre>";
    h += "<script>"
         "let autoScroll=true;"
         "const logEl=document.getElementById('log');"
         "document.getElementById('clear').addEventListener('click', async ()=>{"
         "  await fetch('/clearlog');"
         "});"
         "logEl.addEventListener('scroll', ()=>{"
         "  autoScroll=(logEl.scrollTop + logEl.clientHeight >= logEl.scrollHeight - 5);"
         "});"
         "async function r(){"
         "  let t=await fetch('/logs?raw=1');"
         "  logEl.textContent=await t.text();"
         "  if(autoScroll) logEl.scrollTop=logEl.scrollHeight;"
         "}"
         "setInterval(r,1000); r();"
         "</script>";
    h += "</body></html>";
    server.send(200, "text/html; charset=utf-8", h);
  });

  // Imposta un singolo relè: /relay?i=3&s=1
  server.on("/relay", HTTP_GET, []() {
    if (!server.hasArg("i") || !server.hasArg("s")) {
      server.send(400, "text/plain", "Missing i or s");
      return;
    }

    int i = server.arg("i").toInt();
    int s = server.arg("s").toInt();

    if (i < 0 || i > 15) {
      server.send(400, "text/plain", "Bad index");
      return;
    }

    bool newState = (s != 0);
    relayState[i] = newState;
    if (mcp_ok) mcp.digitalWrite(i, newState ? HIGH : LOW);

    logPrintf("Relay %d -> %s\n", i, newState ? "ON" : "OFF");
    server.send(200, "application/json; charset=utf-8", jsonStatus());
  });

  // Imposta tutti i relè: /relayall?s=1 oppure /relayall?s=0
  server.on("/relayall", HTTP_GET, []() {
    if (!server.hasArg("s")) {
      server.send(400, "text/plain", "Missing s");
      return;
    }
    int s = server.arg("s").toInt();
    bool newState = (s != 0);

    for (int i = 0; i < 16; i++) {
      relayState[i] = newState;
      if (mcp_ok) mcp.digitalWrite(i, newState ? HIGH : LOW);
      yield();
    }

    logPrintf("Relay ALL -> %s\n", newState ? "ON" : "OFF");
    server.send(200, "application/json; charset=utf-8", jsonStatus());
  });

  // OTA (no lambda: nella tua libreria non è supportato)
  httpUpdater.setup(&server, "/update", "admin", "admin123");
  logPrintf("OTA upload: http://%s/update (user=admin)\n", deviceIPString().c_str());
  logPrintf("OTA menu:   http://%s/ota\n", deviceIPString().c_str());

  server.begin();
  logPrintf("Server HTTP avviato.\n");
  logPrintf("Apri: http://%s/\n", deviceIPString().c_str());
}

void loop() {
  server.handleClient();
  yield();
}
