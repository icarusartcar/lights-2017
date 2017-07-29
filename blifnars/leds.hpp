#pragma once

struct CRGB;
struct CHSVS;

#include <string>

void led_init();
void led_run();
bool led_switch(std::string mode);

void lights_solid(const CRGB& color);
void statusblink(uint8_t ntimes, const CRGB& color, uint8_t brightness);
void set(const CHSVS&);

const unsigned int MAX_OCP_CHANNELS = 10;
const unsigned int MAX_LEDS_PER_OCP_CHANNEL = 484;
const unsigned int MAX_LEDS = MAX_OCP_CHANNELS * MAX_LEDS_PER_OCP_CHANNEL;
alignas(32) extern CRGB leds[MAX_LEDS];

const unsigned int DATA_PIN=13;
const unsigned int CLOCK_PIN=14;

struct run_mode {
  virtual void enter_impl();
  virtual void leave_impl();
  virtual int run_impl() = 0;
  void enter();
  void run();
  void leave();
};
