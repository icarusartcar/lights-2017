// #pragma GCC message ignored
//#define DEBUG_ESP_WIFI 1
//#define DEBUG_ESP_PORT Serial

// #define DEBUG_ESP_HTTP_UPDATE
// #define DEBUG_ESP_PORT Serial

#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClient.h>

#include <FS.h>
#include <stdio.h>
#include "leds.hpp"
#include "main.hpp"

#include "globals.hpp"
#include "icarus.hpp"
#include "parameter.hpp"

extern "C" {
// for wifi_station stuffs
#include "user_interface.h"
}

#define FASTLED_INTERNAL
#include "FastLED.h"

const String version("[built " __DATE__ " at " __TIME__ "]");

bool ota_in_progress = 0;

void check_wifi();

ESP8266WebServer server(80);

unsigned relaya_state = 0;
unsigned relayb_state = 0;

const char* statusmsg() {
  static char reply[1024];
  std::string pg = pretty_globals();
  sprintf(reply,
          "\
Version %s\n\
Globals: %s\n\
IP=%s\n\
relaya_state = %d\n\
relayb_state = %d\n\
\n\
Hit http://(ip)/set?name=value to set values\n\
",
          version.c_str(),
	  pg.c_str(),
	  WiFi.localIP().toString().c_str(),
          relaya_state == 0 ? LOW : HIGH,
	  relayb_state == 0 ? LOW : HIGH);
  return reply;
}

void handleRoot() {
  server.send(200, "text/plain", statusmsg());
}

void handleSetMode() {
  iclog("handleSetMode");
  bool success = false;
  if (server.hasArg("mode")) {
    parameter<std::string> pi;
    std::string newmode;
    pi.set(server.arg("mode"));
    newmode = pi;
    success = led_switch(newmode);
  }
  if (success)
    server.send(200, "text/plain", "ok\n");
  else
    server.send(501, "text/plain", "not implemented\n");
}

void handleSet() {
  iclog("handleSet");

  if (server.hasArg("hostname")) {
    hostnamep.set(server.arg("hostname"));
    hostnamep.store();
  }

  if (server.hasArg("loghost")) {
    loghost.set("bird.resophonic.com");
    loghost.store();
  }

  if (server.hasArg("ssid")) {
    ssid.set(server.arg("ssid"));
    ssid.store();
  }

  if (server.hasArg("threewire")) {
    threewire.set(server.arg("threewire"));
    threewire.store();
  }

  if (server.hasArg("opc_channels")) {
    opc_channels.set(server.arg("opc_channels"));
    opc_channels.store();
  }

  if (server.hasArg("pixels_per_channel")) {
    pixels_per_channel.set(server.arg("pixels_per_channel"));
    pixels_per_channel.store();
  }

  if (server.hasArg("brightness")) {
    brightness.set(server.arg("brightness"));
    brightness.store();
  }

  if (server.hasArg("password")) {
    password.set(server.arg("password"));
    password.store();
  }

  if (server.hasArg("relaya_state")) {
    relaya_state = server.arg("relaya_state").toInt();
    digitalWrite(RELAYA_PIN, relaya_state == 0 ? LOW : HIGH);
  }

  if (server.hasArg("relayb_state")) {
    relayb_state = server.arg("relayb_state").toInt();
    digitalWrite(RELAYB_PIN, relayb_state == 0 ? LOW : HIGH);
  }
  handleRoot();
}

void handleReset() {
  iclog("Reset!  Somebody tagged my reset URL /reset, bbiab...");
  server.send(200, "text/plain", "Ok, if you insist, I'll have a reset.  bbiab...\n\n");
  delay(100);  // give that thing a bit to get out on the wire

  statusblink(5, CRGB::Yellow, 255);
  ESP.reset();
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void ota_setup() {
  ArduinoOTA.setHostname(hostnamep.c_str());
  // ArduinoOTA.setPassword("");
  ArduinoOTA.onStart([]() {  // switch off all the PWMs during upgrade
    iclog("OTA start");
    ota_in_progress = true;
  });

  ArduinoOTA.onEnd([]() {  // do a fancy thing with our board led at end
    iclog("OTA end");
    ota_in_progress = false;
    ESP.restart();
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    iclog("OTA progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) { ESP.restart(); });

  // setup the OTA server
  ArduinoOTA.begin();
}

void setup_impl(void) {
  Serial.begin(115200);
  iclog("Starting v%u", 5);
  iclog("Setting output pins...");

  // icrs_udp_init();

  relaya_state = 0;
  digitalWrite(RELAYA_PIN, relaya_state);
  pinMode(RELAYA_PIN, OUTPUT);

  relayb_state = 0;
  digitalWrite(RELAYB_PIN, relayb_state);
  pinMode(RELAYB_PIN, OUTPUT);

  pinMode(BLUELED_PIN, OUTPUT);
  digitalWrite(BLUELED_PIN, HIGH);  // off
  pinMode(REDLED_PIN, OUTPUT);
  digitalWrite(REDLED_PIN, HIGH);  // off
  iclog("done.");

  iclog("Mounting filesystem...");
  if (!SPIFFS.begin()) {
    iclog("FAILED.");
    return;
  } else {
    iclog("mounted.  Filesystem contains:");
    Dir dir = SPIFFS.openDir("");
    while (dir.next()) {
      iclog("  %s", dir.fileName().c_str());
      File f = dir.openFile("r");
      iclog("size=%d", f.size());
    }
  }

  register_globals();

  led_init();
  led_switch("unicorn_puke");

  iclog("getting ready to check wifi");
  check_wifi();

  iclog("BOOTED %s", version.c_str());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/setmode", HTTP_GET, handleSetMode);
  server.on("/set", HTTP_GET, handleSet);
  server.on("/reset", HTTP_GET, handleReset);

  server.onNotFound(handleNotFound);

  server.begin();
  iclog("HTTP server started");

  ota_setup();

  statusblink(10, CRGB::Green, 255);
}

void flash_red() {
  static bool state = 0;
  state = !state;
  digitalWrite(REDLED_PIN, state);
}

void flash_blue() {
  static bool state = 0;
  state = !state;
  digitalWrite(BLUELED_PIN, state);
}

static_assert(HIGH == 1, "uh oh");
static_assert(LOW == 0, "uh oh");

void check_wifi() {
  if (WiFi.status() == WL_CONNECTED) return;

  iclog("Using hostname %s", hostnamep.c_str());
  WiFi.mode(WIFI_STA);  // or WIFI_AP or WIFI_AP_STA
  iclog("WIFITIME");
  // WiFi.hostname(hostnamep.c_str());

  WiFi.setPhyMode(WIFI_PHY_MODE_11G);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.mode(WIFI_STA);

  /*wl_status_t stat =*/WiFi.begin(ssid.c_str(), password.c_str());

  WiFi.setAutoConnect(false);
  WiFi.setAutoReconnect(false);

  bool wetried = false;
  static const char* station_status_str[] = {
      "STATION_IDLE",        "STATION_CONNECTING",   "STATION_WRONG_PASSWORD",
      "STATION_NO_AP_FOUND", "STATION_CONNECT_FAIL", "STATION_GOT_IP"};

  led_switch("unicorn_puke");

  while (WiFi.status() != WL_CONNECTED) {
    wetried = true;
    led_run();
    yield();
    EVERY_N_SECONDS(1) {
      station_status_t status = wifi_station_get_connect_status();
      iclog("status = %d (%s)", status, station_status_str[status]);
    }
    EVERY_N_SECONDS(3) {
      iclog("status=%d", WiFi.status());
      WiFi.printDiag(Serial);
    }
    yield();
    flash_red();
    yield();
    // statusblink(1, CRGB::Red);
    EVERY_N_SECONDS(15) {
      iclog(".Hrm, reconnecting...");
      WiFi.reconnect();
    }
    yield();
  }
  if (wetried)  // we reconnected
  {
    iclog("Connected to: %s", ssid.c_str());
    String mac = WiFi.macAddress();
    iclog("Mac address:  %s", mac.c_str());
    iclog("IP  address:  %s", WiFi.localIP().toString().c_str());
    WiFi.printDiag(Serial);

    statusblink(10, CRGB::Green, 255);
  }
}

void loop_impl(void) {
  if (!ota_in_progress) {

    EVERY_N_SECONDS(1) { flash_blue(); }

    EVERY_N_MILLISECONDS(50) { server.handleClient(); }

    EVERY_N_MILLISECONDS(1591) { check_wifi(); }

    led_run();
    yield();
  }
  ArduinoOTA.handle();
}
