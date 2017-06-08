#ifndef DEBUG_PROFILER_H
#define DEBUG_PROFILER_H

#include <string>
#include <vector>

#include <nds.h>

namespace debug {

struct TimingResult {
  u32 start = 0;
  u32 end = 0;
  u32 delta() {
    return end - start;
  }
};

struct Topic {
  std::string name;
  u16 rgb5_color;
  TimingResult timing;
};

namespace Profiler {

void StartTopic(int topic_id);
void EndTopic(int topic_id);
void ClearTopic(int topic_id);

void StartTimer();

int RegisterTopic(std::string name, u16 rgb5_color = 0xFFFF);
int GetTopicByName(std::string name);
std::vector<Topic>& Topics();

} // namespace Profiler

} // namespace debug

#endif // DEBUG_PROFILER_H
