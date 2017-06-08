#ifndef DEBUG_FLAGS_H
#define DEBUG_FLAGS_H

#include <map>
#include <string>

namespace debug {
  void RegisterFlag(std::string name);
  bool Flag(std::string name);
  void SetFlag(std::string name, bool value);
  void ClearFlag(std::string name, bool value);
  void ToggleFlag(std::string name, bool value);
  std::map<std::string, bool>& FlagList();
} // namespace debug

#endif
