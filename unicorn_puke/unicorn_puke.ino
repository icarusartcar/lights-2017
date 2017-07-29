#include "FastLED.h"

// the standard preamble

const unsigned int MAX_LEDS=200;
const unsigned int DATA_PIN=13;
const unsigned int CLOCK_PIN=14;
alignas(32) CRGB leds[MAX_LEDS];

void setup() {
  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR>(leds, MAX_LEDS)
    .setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(255);
}

// use whatever blinkiness you like here.  this is from one of the
// FastLED demo loops.

uint8_t hue = 0;

void loop() {
  EVERY_N_MILLISECONDS(1) {
    hue += 1;
    fill_rainbow(leds, MAX_LEDS, hue, 3);
  }
  FastLED.show();
}
