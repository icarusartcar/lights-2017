#pragma once

#include <memory>
#include <set>
#include "parameter.hpp"

extern parameter<uint8_t> threewire;
extern parameter<uint8_t> brightness;
extern parameter<uint8_t> opc_channels;
extern parameter<uint16_t> pixels_per_channel;

extern parameter<std::string> loghost;
extern parameter<std::string> hostnamep;
extern parameter<std::string> ssid;
extern parameter<std::string> password;

extern std::set<parameter_base*> parameters;


void register_globals();
std::string pretty_globals();
