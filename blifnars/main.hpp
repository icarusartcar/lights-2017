#pragma once

#include <cstdint>

#include "leds.hpp"

const static uint8_t RELAYA_PIN=14;
const static uint8_t RELAYB_PIN=12;
const static uint8_t LEDA_PIN=5;
const static uint8_t LEDB_PIN=4;
const static uint8_t BLUELED_PIN=2;
const static uint8_t REDLED_PIN=0;


extern unsigned int relaya_state, relayb_state;

void setup_impl();
void loop_impl();
