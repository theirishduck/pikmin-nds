#ifndef DEBUG_DICTIONARY_H
#define DEBUG_DICTIONARY_H

#include <string>
#include <map>

#include "numeric_types.h"
#include "vector.h"

namespace debug {

class Dictionary {
public:
  Dictionary();
  ~Dictionary();

  void Set(const std::string &name, int value);
  void Set(const std::string &name, numeric_types::fixed value);
  void Set(const std::string &name, Vec3 value);
  void Set(const std::string &name, const std::string value);

  std::map<std::string, std::string> DisplayValues();

private:
  std::map<std::string, int> ints_;
  std::map<std::string, numeric_types::fixed> fixeds_;
  std::map<std::string, Vec3> vectors_;
  std::map<std::string, std::string> strings_;
};

} // namespace debug

#endif // DEBUG_DICTIONARY_H
