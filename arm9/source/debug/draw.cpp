#include "debug/draw.h"
#include "debug/flags.h"
#include "physics/world.h"

namespace debug {

physics::World* world_;

void RegisterWorld(physics::World* world) {
  world_ = world;
}

void DrawPhysicsCircles() {
  world_->DebugCircles();
}

void DrawEffects() {
  if (debug::Flag("Draw Physics Circles")) {
    DrawPhysicsCircles();
  }

}

} // namespace debug
