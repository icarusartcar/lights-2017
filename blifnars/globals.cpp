#include <vector>
#include <FS.h>

#include "icarus.hpp"
#include "globals.hpp"

parameter<uint8_t> threewire("threewire", 0);
parameter<uint8_t> brightness("brightness", 255);
parameter<uint8_t> opc_channels("opc_channels", 5);
parameter<uint16_t> pixels_per_channel("pixels_per_channel", 484);
parameter<std::string> loghost("loghost", "loghost");
parameter<std::string> hostnamep("hostname", "dummy-hostname");
parameter<std::string> ssid("ssid", "outerspace");
parameter<std::string> password("password", "outerspace");

std::set<parameter_base*> parameters;


void register_globals() {
  parameters.insert(&threewire);
  parameters.insert(&brightness);
  parameters.insert(&opc_channels);
  parameters.insert(&pixels_per_channel);
  parameters.insert(&loghost);
  parameters.insert(&hostnamep);
  parameters.insert(&ssid);
  parameters.insert(&password);

  for (auto& p : parameters) {
    p->load();
  }
  iclog("%s", pretty_globals().c_str());
}

std::string pretty_globals() {
  std::string buf;
  for (auto& p : parameters) {
    buf += p->prettyprint();
    buf += "\r\n";
  }
  return buf;
}
