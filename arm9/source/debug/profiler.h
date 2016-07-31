#ifndef DEBUG_PROFILER_H
#define DEBUG_PROFILER_H

#include <nds.h>
#include <string>
#include <vector>

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

class Profiler {
public:
  void StartTopic(int topic_id);
  void EndTopic(int topic_id);
  void ClearTopic(int topic_id);

  void StartTimer();

  int RegisterTopic(std::string name, u16 rgb5_color = 0xFFFF);
  int GetTopicByName(std::string name);
  std::vector<Topic>& Topics();

private:
  std::vector<Topic> topics_;
};

} // namespace debug

#endif // DEBUG_PROFILER_H
