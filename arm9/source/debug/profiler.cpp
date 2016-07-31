#include "debug/profiler.h"

namespace debug {

void Profiler::StartTopic(int topic_id) {
  topics_[topic_id].timing.start = cpuGetTiming();
}

void Profiler::EndTopic(int topic_id) {
  topics_[topic_id].timing.end = cpuGetTiming();
}

void Profiler::ClearTopic(int topic_id) {
  topics_[topic_id].timing.start = 0;
  topics_[topic_id].timing.end = 0;
}

void Profiler::StartTimer() {
  cpuStartTiming(0);
}

int Profiler::RegisterTopic(std::string name, u16 rgb5_color) {
  int new_id = topics_.size();
  Topic new_topic;
  new_topic.name = name;
  new_topic.rgb5_color = rgb5_color;
  topics_.push_back(new_topic);
  return new_id;
}

int Profiler::GetTopicByName(std::string name) {
  for (unsigned int i = 0; i < topics_.size(); i++) {
    if (topics_[i].name == name) {
      return i;
    }
  }
  return -1;
}

std::vector<Topic>& Profiler::Topics() {
  return topics_;
}

} // namespace debug
