#include <map>
#include <memory>
#include <typeindex>

#include <ESP8266WiFi.h>
#include <lwip/stats.h>

#include "globals.hpp"
#include "icarus.hpp"
#include "leds.hpp"
#include "main.hpp"

#define FASTLED_ESP8266_RAW_PIN_ORDER
// #define FASTLED_INTERNAL
#include "FastLED.h"

FASTLED_USING_NAMESPACE

using namespace std;

struct opc_mode : run_mode {
  ~opc_mode() = default;

  WiFiServer server{7890};
  WiFiClient client;

  uint8_t channel{0}, command{0};
  uint16_t data_length;
  uint32_t n_packets{0};

  int run_impl()
  {
    bool nodelay = false;
    if (!client) {
      // Check if a client has connected
      client = server.available();
      if (!client) {
	return 0;
      }
      client.setNoDelay(nodelay);
      iclog("new OPC client");
    }

    if (!client.connected()) {
      iclog("OPC client disconnected");
      client = server.available();
      if (!client) {
	return 0;
      }
      client.setNoDelay(nodelay);
    }

    if (client.available() == 0)
      return 0;

    int r;

    r = client.read();
    if (r == -1)
      return 0;
    channel = r;

    if (channel > opc_channels)
      iclog("error, send to channel %d of %d", channel, opc_channels.get());

    r = client.read();
    if (r == -1)
      return 0;
    command = r;

    r = client.read();
    if (r == -1)
      return 0;
    data_length = r << 8;

    r = client.read();
    if (r == -1)
      return 0;
    data_length |= r;

    n_packets++;
    //unsigned npixels = data_length / 3;

    //iclog("state: %u chan: %u cmd: %u data_length: %u",
    //	  client.status(), channel, command, data_length);

    int actually_read = 0;
    int n_zeros = 0;

    CRGB* leds_start = leds + (channel-1) * pixels_per_channel.get();
    // special case
    if (channel == 0)
      leds_start = leds;

    while (actually_read < data_length) {
      int thisread_bytes = client.read((uint8_t*)(leds_start + actually_read), data_length - actually_read);
      if (thisread_bytes == 0)
	n_zeros++;
      actually_read += thisread_bytes;
      //iclog("actually_read: %u", actually_read);
      if (n_zeros > 5) {
	iclog("zero read... packet too big? at %d bytes of %d\navailable=%d connected=%d status=%d", actually_read, data_length,
	      client.available(), client.connected(), client.status());
	client.stop();
	return 0;
      }
    }
    if (channel == 0) {
      for (int j=1; j<=opc_channels.get(); ++j) {
	memcpy(leds + j * pixels_per_channel.get(), leds, pixels_per_channel.get() * sizeof(CRGB));
      }
    }
    static unsigned k = 0;
    k++;
    static const char* const spinner = "-\\|/";

    EVERY_N_SECONDS(5) {
      iclog("n_packets: %u", n_packets);
    }
    return 1;
  }

  void enter_impl() {
    server.begin();
  }

  void leave_impl() {
    client.stop();
    server.stop();
  }
};


std::shared_ptr<run_mode> make_opc_mode() {
  return std::make_shared<opc_mode>();
}
