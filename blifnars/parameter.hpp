#pragma once

#include "icarus.hpp"
#include <FS.h>
#include <HardwareSerial.h>

#include <type_traits>
#include <cstdint>

struct parameter_base {
  virtual const char* prettyprint() = 0;
  virtual int load() = 0;
};

template <typename T>
class parameter : public parameter_base {

  struct stringize_visitor {
    std::string operator()(std::string s) { return s; }
    std::string operator()(uint8_t uc) {
      char buf[255];
      snprintf(buf, 255, "%u", uc);
      return std::string(buf);
    }
    std::string operator()(uint16_t uc) {
      char buf[255];
      snprintf(buf, 255, "%u", uc);
      return std::string(buf);
    }
  };

public:
  parameter() { }

  parameter(const char* name, T defvalue) : name_(name), val_(defvalue)
  {
    spiffsname_ = "/";
    spiffsname_ += name_;
  }

  T get() { return val_; }
  const char* name() { return name_.c_str(); }

  void set(T newvalue)
  {
    val_ = newvalue;
  }

  void set_integral_impl(const char* buf, std::true_type)
  {
    val_ = atoi(buf);
  }

  void set_integral_impl(const char* buf, std::false_type) { }

  void set_string_impl(const char* buf, std::true_type)
  {
    val_ = buf;
  }

  void set_string_impl(const char* buf, std::false_type) { }

  void set(const char* newvalue)
  {
    // these have to be mutually exclusive
    set_integral_impl(newvalue, typename std::is_integral<T>());
    set_string_impl(newvalue, typename std::is_same<std::string,T>());
  }

  void set(String newvalue)
  {
    // these have to be mutually exclusive
    set(newvalue.c_str());
  }

  operator T() const
  {
    return val_;
  }

  const char* prettyprint()
  {
    stringize_visitor sv;
    pretty_ = name();
    pretty_ += " = ";
    pretty_ += sv(val_);
    return pretty_.c_str();
  }

  /*
  template <typename U>
  const char* c_str_dispatch(const U&, std::false_type) const
  {
    static_assert(0, "you can't c_str() this type");
    return 0;
  }
  */
  template <typename U>
  const char* c_str_dispatch(const U& s,
                             typename std::enable_if<std::is_same<U, std::string>::value>::type* t = 0) const
  {
    return s.c_str();
  }

  const char* c_str() const {
    return c_str_dispatch(val_);
  }

  template <typename U>
  int read(File& f, U& value)
  {
    iclog("reading %u bytes", sizeof(U));
    // if our file is too small (type size has changed), then just use default
    if(f.size() != sizeof(U))
      return -1;

    return f.read(reinterpret_cast<uint8_t*>(&value), sizeof(U));
  }

  int read(File& f, std::string& s)
  {
    // if our file is too small (type size has changed), then just use default
    if(f.size() != 255)
      return -1;

    char buf[255];
    memset(buf, 0, 255);

    iclog("reading %u bytes", 255);
    int b = f.read(reinterpret_cast<uint8_t*>(&buf), 255);
    s = buf;
    return b;
  }

  int load() {

    iclog("trying load from %s", spiffsname_.c_str());

    if (! SPIFFS.exists(spiffsname_.c_str())) {
      iclog("file does not exist");
      return -1;
    }
    File f = SPIFFS.open(spiffsname_.c_str(), "r");

    int b = read(f, val_);
    iclog("%s: loaded %u bytes from %s", name_.c_str(), b, spiffsname_.c_str());
    f.close();
    return b;
  }

  template <typename U>
  int write(File& f, const U& u)
  {
    return f.write(reinterpret_cast<const uint8_t*>(&u), sizeof(u));
  }

  int write(File& f, const std::string& u)
  {
    return f.write(reinterpret_cast<const uint8_t*>(u.c_str()), u.size());
  }

  void store() {
    iclog("storing %s to %s", name_.c_str(), spiffsname_.c_str());
    File f = SPIFFS.open(spiffsname_.c_str(), "w");
    if (!f) {
      Serial.printf("Couldn't open %s for writing\n", spiffsname_.c_str());
      return;
    }
    int b = write(f, val_);
    iclog("%s: wrote %u bytes to %s", name_.c_str(), b, spiffsname_.c_str());
    f.flush();
    f.close();
  }

private:

  std::string name_, spiffsname_, pretty_;
  T val_;

};
