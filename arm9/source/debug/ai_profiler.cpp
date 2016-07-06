#include "debug/ai_profiler.h"

namespace debug {

void AiProfiler::ClearTimingData() {
  for (auto& kv : state_timings_) {
    auto& state = kv.second;
    state.start = 0;
    state.end = 0;
    state.total = 0;
    state.run_count = 0;
    for (auto& edge : state.edges) {
      edge.start = 0;
      edge.end = 0;
      edge.total = 0;
      edge.run_count = 0;
    }
  }
}

void AiProfiler::StartState(std::string &node_name) {
  // Make sure there's a timer for this node in place
  state_timings_.emplace(node_name, StateTiming());
  state_timings_[node_name].start = cpuGetTiming();
}

void AiProfiler::EndState(std::string &node_name) {
  state_timings_[node_name].end = cpuGetTiming();
  state_timings_[node_name].total += state_timings_[node_name].end - state_timings_[node_name].start;
  state_timings_[node_name].run_count++;
}

void AiProfiler::StartEdge(std::string &node_name, unsigned int edge_index) {
  // Ensure this edge list is large enough to hold the incoming index. Resize otherwise.
  if (state_timings_[node_name].edges.size() <= edge_index) {
    state_timings_[node_name].edges.resize(edge_index + 1);
  }
  state_timings_[node_name].edges[edge_index].start = cpuGetTiming();
}

void AiProfiler::EndEdge(std::string &node_name, unsigned int edge_index) {
  state_timings_[node_name].edges[edge_index].end = cpuGetTiming();
  state_timings_[node_name].edges[edge_index].total =
    state_timings_[node_name].edges[edge_index].end -
    state_timings_[node_name].edges[edge_index].start;
  state_timings_[node_name].edges[edge_index].run_count++;
}

std::map<std::string, StateTiming>& AiProfiler::StateTimings() {
  return state_timings_;
}

} // namespace debug
