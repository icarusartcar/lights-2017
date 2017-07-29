#include <map>
#include <memory>
#include <typeindex>

#include "globals.hpp"
#include "icarus.hpp"
#include "leds.hpp"
#include "opc_mode.hpp"
#include "main.hpp"

#define FASTLED_ESP8266_RAW_PIN_ORDER
// #define FASTLED_INTERNAL
#include "FastLED.h"

FASTLED_USING_NAMESPACE

alignas(32) CRGB leds[MAX_LEDS];

using namespace std;

void run_mode::enter_impl() { }
void run_mode::leave_impl() { }

void run_mode::enter() {
  FastLED.setBrightness(brightness);
  for (auto& led : leds) {
    led = CRGB::Black;
  }
  this->enter_impl();
  FastLED.show();
}

void run_mode::run()
{
  FastLED.setBrightness(brightness);
  if (this->run_impl() != 0)
    FastLED.show();
}

void run_mode::leave() {
  iclog("run_mode::leave()");
  this->leave_impl();
}


struct heartbeat_mode : run_mode {
  ~heartbeat_mode() = default;

  int run_impl() {
    static bool onoff = false;
    EVERY_N_MILLISECONDS(800) {
      onoff = !onoff;
    }
    auto clr = onoff ? CRGB::Red : CRGB::Black;
    for (unsigned i = 0; i < MAX_LEDS; i++) {
      leds[i] = clr;
    }
    return 1;
  }
};

struct unicorn_puke_mode : run_mode {
  ~unicorn_puke_mode() = default;

  uint8_t hue{0};

  void enter_impl() {
    brightness.set(255);
    FastLED.setBrightness(brightness);
  }

  int run_impl()
  {
    EVERY_N_MILLISECONDS(1) {
      hue += 5;
      fill_rainbow(leds, opc_channels.get() * pixels_per_channel.get(), hue, 3);
      return 1;
    }
    return 0;
  }
};

struct dots_mode : run_mode {
  ~dots_mode() = default;

  uint8_t hue{0};
  int run_impl() {

    EVERY_N_MILLISECONDS(10) {
      ++hue;

      // a colored dot sweeping back and forth, with fading trails
      fadeToBlackBy(leds, opc_channels.get() * pixels_per_channel.get(), 20);
      int pos = beatsin16(13, 0, opc_channels.get() * pixels_per_channel.get());
      leds[pos] += CHSV(hue, 255, 192);
      return 1;
    }
    return 0;
  }
};

struct soothe_mode : run_mode {
  ~soothe_mode() = default;

  void enter_impl() {
    for (unsigned i = 0; i < MAX_LEDS; i++) {
      leds[i] = CRGB::Red;
    }
  }
  int run_impl() {
    return 1;
  }
};

struct cops_mode : run_mode {
  ~cops_mode() = default;
  bool flag{false};

  int run_impl() {
    EVERY_N_MILLISECONDS(250) {
      flag = !flag;
      for (unsigned i = 0; i < MAX_LEDS; i++) {
	leds[i] = flag ? CRGB::Red : CRGB::Blue;
      }
      return 1;
    }
    return 0;
  }
};

std::map<std::string, std::shared_ptr<run_mode>> modemap;
std::shared_ptr<run_mode> current_mode;

void led_init() {
  // TODO: this should be runtime-configurable to support either three
  // wire or four-wire strips.

  // four wire version
  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR>(leds, opc_channels.get() * pixels_per_channel.get())
      .setCorrection(TypicalLEDStrip);

  modemap["soothe"] = make_shared<soothe_mode>();
  modemap["heartbeat"] = make_shared<heartbeat_mode>();
  modemap["cops"] = make_shared<cops_mode>();
  modemap["dots"] = make_shared<dots_mode>();
  modemap["unicorn_puke"] = make_shared<unicorn_puke_mode>();
  modemap["opc"] = make_opc_mode();

  iclog("added leds");
  statusblink(5, CRGB::DimGrey, 255);
}

void led_run() {
  if (current_mode)
    current_mode->run();
}

bool led_switch(std::string newmode) {
  iclog("switch to mode %s", newmode.c_str());
  if (modemap.count(newmode)) {
    if (current_mode)
      current_mode->leave();
    current_mode = modemap[newmode];
    current_mode->enter();
    return true;
  } else {
    iclog("attempt to switch to unknown mode %s ignored", newmode.c_str());
    return false;
  }
}

void lights_solid(const CRGB& color) {
  fill_solid(leds, opc_channels.get() * pixels_per_channel.get(), color);
  FastLED.show();
}

void statusblink(uint8_t ntimes, const CRGB& color, uint8_t brightness) {
  FastLED.setBrightness(brightness);
  for (int i = 0; i < ntimes; ++i) {
    lights_solid(color);
    delay(2);
    lights_solid(CRGB::Black);
    if (i + 1 > ntimes) {
      delay(98);
    }
  }
  FastLED.setBrightness(brightness);
}
