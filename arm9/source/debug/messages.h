#ifndef DEBUG_MESSAGES_H
#define DEBUG_MESSAGES_H

#include <deque>
#include <string>

namespace debug {

void Log(std::string message);
std::deque<std::string> Messages();

} // namespace debug

#endif
