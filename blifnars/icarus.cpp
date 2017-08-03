#include <ESP8266WiFi.h>
#include <FS.h>
#include <string>
#include "icarus.hpp"

// extern WiFiUDP syslog_udp;

template <typename T>
void fs_store(String name, T value)
{
  File f = SPIFFS.open(name, "w");
  if (!f) {
    iclog("Couldn't open %s for writing\n", name.c_str());
    return;
  }
  int b = f.write(reinterpret_cast<uint8_t*>(&value), sizeof(T));
  iclog("wrote %d bytes for %s\n", b, name.c_str());
  f.flush();
  f.close();
}

template <>
void fs_store<String>(String name, String value)
{
  File f = SPIFFS.open(name, "w");
  if (!f) {
    iclog("Couldn't open %s for writing", name.c_str());
    return;
  }
  int b = f.write(reinterpret_cast<const uint8_t*>(value.c_str()), value.length());
  iclog("wrote %d bytes for %s\n", b, name.c_str());
  f.flush();
  f.close();
}

template void fs_store(String, int);
template void fs_store(String, unsigned char);

template <typename T>
T fs_retrieve(String name, T defvalue)
{
  if (! SPIFFS.exists(name)) {
    iclog("%s not set, using default of ", name.c_str());
    return defvalue;
  }

  File f = SPIFFS.open(name, "r");
  T value;
  int b = f.read(reinterpret_cast<uint8_t*>(&value), sizeof(T));
  iclog("%s was %d bytes on disk\n", name.c_str(), b);
  f.close();
  return value;
}

template <>
String fs_retrieve<String>(String name, String defvalue)
{
  if (! SPIFFS.exists(name)) {
    Serial.println(name + " not set, using default of ");
    Serial.println(defvalue);
    return defvalue;
  }

  File f = SPIFFS.open(name, "r");
  char buf[256];
  memset(buf, 0, 256);
  int b = f.read(reinterpret_cast<uint8_t*>(&buf), 256);
  iclog("%s was %d bytes on disk\n", name.c_str(), b);
  String s(buf);
  f.close();
  return s;
}

template int fs_retrieve(String, int);
template unsigned char fs_retrieve(String, unsigned char);
