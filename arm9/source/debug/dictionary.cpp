#include "debug/dictionary.h"

#include <cstdio>

using numeric_types::fixed;

namespace debug {

Dictionary::Dictionary() {
}

Dictionary::~Dictionary() {
}

void Dictionary::Set(const std::string &name, int value) {
  ints_[name] = value;
}

void Dictionary::Set(const std::string &name, fixed value) {
  fixeds_[name] = value;
}

void Dictionary::Set(const std::string &name, Vec3 value) {
  vectors_[name] = value;
}

void Dictionary::Set(const std::string &name, std::string value) {
  strings_[name] = value;
}

std::map<std::string, std::string> Dictionary::DisplayValues() {
  char buffer[256];
  std::map<std::string, std::string> values;
  for (auto kv : ints_) {
    std::sprintf(buffer, "%d", kv.second);
    values[kv.first] = std::string(buffer);
  }

  for (auto kv : fixeds_) {
    std::sprintf(buffer, "%.3f", (float)kv.second);
    values[kv.first] = buffer;
  }

  for (auto kv : vectors_) {
    std::sprintf(buffer, "(%.1f, %.1f, %.1f)", (float)kv.second.x, (float)kv.second.y, (float)kv.second.z);
    values[kv.first] = buffer;
  }

  for (auto kv : strings_) {
    values[kv.first] = kv.second;
  }

  return values;
}

}
