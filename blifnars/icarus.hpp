#pragma once

#include <FS.h>
#include <string>
#undef max

#define stringify(x) #x
#define tostring(x) stringify(x)

template <typename... Ts>
int iclog_impl(const char* pretty_function, const char* fmt, const Ts&... ts) {
  constexpr int MAX = 255;
  static char buf[MAX];

  int n = snprintf(buf, MAX, "%s: ", pretty_function);

  n += snprintf(buf + n, MAX - n, fmt, ts...);
  snprintf(buf + n, MAX - n, "\n\r");

  Serial.print(buf);

  //  yield();
  //  syslog_udp.beginPacket("bridge", 514);
  //  syslog_udp.printf(fmt, ts...);
  //  syslog_udp.endPacket(;)
  return n;
}

#ifndef NOLOG
#define iclog(FMT, ...) iclog_impl(__PRETTY_FUNCTION__, FMT, ##__VA_ARGS__)
#else
#define iclog(FMT, ...)
#endif

#define iccheck(COND, FMT, ...) \
  (void)((COND) || iclog_impl(__PRETTY_FUNCTION__, FMT, ##__VA_ARGS__))

template <typename T>
void fs_store(String name, T value);

template <typename T>
T fs_retrieve(String name, T defvalue);
