#include <map>
#include <memory>
#include <typeindex>

#include <ESP8266WiFi.h>

#include "globals.hpp"
#include "icarus.hpp"
#include "leds.hpp"
#include "main.hpp"

#define FASTLED_ESP8266_RAW_PIN_ORDER
// #define FASTLED_INTERNAL
#include "FastLED.h"

FASTLED_USING_NAMESPACE

using namespace std;

std::shared_ptr<run_mode> make_opc_mode();
