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

//** run_mode
// Interface for structs that display a mode.
// Extend and impliment the enter_impl() and leave_imp() 
// functions in order to jive with the leds

void run_mode::enter_impl() { }
void run_mode::leave_impl() { }



// root level entrance function
// Gets the number of leds from channels * pixels per then calls imp
void run_mode::enter() {
  N = opc_channels.get() * pixels_per_channel.get();
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
    FastLED.setBrightness(brightness);
  }

  int run_impl()
  {
    EVERY_N_MILLISECONDS(1) {
      hue -= 5;
      fill_rainbow(leds, N, hue, 3);
      return 1;
    }
    return 0;
  }
};

struct dots_mode : run_mode {
  ~dots_mode() = default;

  CRGB clr = CRGB::Red;
  //  uint8_t hue{192};
  uint16_t pos{0};
  uint8_t n_particles{7};

  int run_impl() {

    EVERY_N_MILLISECONDS(91) {
      ++pos;
    }
    EVERY_N_MILLISECONDS(10) {
      // a colored dot sweeping back and forth, with fading trails
      fadeLightBy(leds, N, 2);
      for (int k=0; k<n_particles; ++k) {
	int this_pos = pos + k*(N/n_particles);
	//	leds[this_pos%N] += CHSV(hue, 255, 255);
	leds[this_pos%N] += clr;
      }
    }
    return 1;
  }
};

struct dotsweep_mode : run_mode {
  ~dotsweep_mode() = default;

  uint8_t hue{0};
  int pxl{0};

  int run_impl() {

    EVERY_N_MILLISECONDS(30) {
      ++hue;
    }
    EVERY_N_MILLISECONDS(20) {
      ++pxl;

      fill_solid(leds, N, CRGB::Black);

      leds[pxl % N] = CHSV(hue, 255, 192);
      blur1d(leds, N, 1000);
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

  int N = opc_channels.get() * pixels_per_channel.get();

  // four wire version
  if (threewire.get() == 1) {
    FastLED.addLeds<WS2811, LEDA_PIN, BGR>(leds, N)
      .setCorrection(TypicalLEDStrip);
    FastLED.addLeds<WS2811, LEDB_PIN, BGR>(leds, N)
      .setCorrection(TypicalLEDStrip);
  } else {
    FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR>(leds, N)
      .setCorrection(TypicalLEDStrip);
  }

  modemap["soothe"] = make_shared<soothe_mode>();
  modemap["heartbeat"] = make_shared<heartbeat_mode>();
  modemap["cops"] = make_shared<cops_mode>();
  modemap["dots"] = make_shared<dots_mode>();
  modemap["dotsweep"] = make_shared<dotsweep_mode>();
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
    mode.set(newmode);
    mode.store();
    return true;
  } else {
    iclog("attempt to switch to unknown mode %s ignored", newmode.c_str());
    return false;
  }
}

void lights_solid(const CRGB& color) {
  fill_solid(leds, MAX_LEDS, color);
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
