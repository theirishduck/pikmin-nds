#ifndef DEBUG_AI_PROFILER_H
#define DEBUG_AI_PROFILER_H

#include <nds.h>

#include <map>
#include <string>
#include <vector>

namespace debug {

struct EdgeTiming {
  u32 start{0};
  u32 end{0};
  u32 total{0};
  u32 run_count{0};
};

struct StateTiming {
  u32 start{0};
  u32 end{0};
  u32 total{0};
  u32 run_count{0};
  std::vector<EdgeTiming> edges;
};

class AiProfiler {
public:
  void ClearTimingData();
  void SetAiType(std::string &ai_name);
  void StartState(std::string &node_name);
  void EndState(std::string &node_name);
  void StartEdge(std::string &node_name, unsigned int edge_index);
  void EndEdge(std::string &node_name, unsigned int edge_index);

  std::map<std::string, StateTiming>& StateTimings();
private:
  std::map<std::string, StateTiming> state_timings_;
};

} // namespace debug

#endif // DEBUG_AI_PROFILER_H
