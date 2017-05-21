#include "messages.h"
#include <nds.h>
#include <deque>

using std::string;

namespace debug {

std::deque<string> messages;

void Log(string message) {
  // Add this message to our internal deque
  messages.push_back(message);
  if (messages.size() > 32) {
    messages.pop_front();
  }

  // Pass this message to nocashMessage, which will print it
  // to stdout in desmume, and collect it in a GUI in no$gba
  nocashMessage(("[LOG] " + message + "\n").c_str());

  // TODO: Write this message out to a file on the SD card? No real need to
  // implement this right away, but could be useful down the line.
}

std::deque<std::string> Messages() {
  return messages;
}

} // namespace debug
