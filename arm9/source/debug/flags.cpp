#include "debug/flags.h"

namespace debug {

std::map<std::string, bool> debug_flags_;

void RegisterFlag(std::string name) {
  debug_flags_[name] = false;
}

bool Flag(std::string name) {
  return debug_flags_[name];
}

void SetFlag(std::string name) {
  debug_flags_[name] = true;
}

void ClearFlag(std::string name) {
  debug_flags_[name] = false;
}

void ToggleFlag(std::string name) {
  debug_flags_[name] = not debug_flags_[name];
}

std::map<std::string, bool>& FlagList() {
  return debug_flags_;
}

} // namespace debug
