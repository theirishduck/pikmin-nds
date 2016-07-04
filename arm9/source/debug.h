#ifndef DEBUG_DRAW_H
#define DEBUG_DRAW_H

#include <nds/arm9/video.h>
#include <nds/arm9/videoGL.h>

#include "vector.h"
#include "numeric_types.h"
#include <string>
#include <sstream>

class PikminGame;

namespace debug {

//some quick functions to make nocash debugging a bit easier
void nocashNumber(int num);

template <typename T>
void nocashValue(std::string name, T value) {
  std::ostringstream oss;
  oss << name << ": " << value;
  nocashMessage(oss.str().c_str());
}

void DrawCrosshair(Vec3 p, rgb color = RGB5(31, 31, 31));
void DrawGroundPlane(int width, int segments, rgb color = RGB5(0, 0, 0));
void DrawCircle(Vec3 p, numeric_types::fixed radius, rgb color, u32 segments = 8);
void DrawLine(Vec3 p1, Vec3 p2, rgb color);
void DrawLine(Vec2 p1, Vec2 p2, rgb color);
void _TimingColor(rgb color);
void Update();
void UpdateTimingMode();
void UpdateTogglesMode();
void InitializeSpawners();
void UpdateSpawnerMode(PikminGame* game);
void AddToggle(std::string name, bool* toggle);
void PrintTitle(const char* title);

enum Topic {
  kPhysics = 0,
  kAI,
  kEntityUpdate,
  kFrameInit,
  kPassInit,
  kPass1,
  kPass2,
  kPass3,
  kPass4,
  kPass5,
  kPass6,
  kPass7,
  kPass8,
  kPass9,
  kIdle,
  kUi,
  kParticleUpdate,
  kParticleDraw,
  kNumTopics
};

void StartTopic(Topic topic);
void EndTopic(Topic topic);
void ClearTopic(Topic topic);
void StartCpuTimer();
void NextTopic();
void PreviousTopic();
void UpdateTopic();

}  // namespace debug

#endif  // DEBUG_DRAW_H
