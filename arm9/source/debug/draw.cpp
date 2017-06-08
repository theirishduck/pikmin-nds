#include "debug/draw.h"
#include "debug/flags.h"
#include "physics/world.h"
#include "render/multipass_renderer.h"

namespace debug {

physics::World* world_;
MultipassRenderer* renderer_;

void RegisterWorld(physics::World* world) {
  world_ = world;
}

void RegisterRenderer(MultipassRenderer* renderer) {
  renderer_ = renderer;
}

void DrawPhysicsCircles() {
  world_->DebugCircles();
}

void DrawRendererCircles() {
  renderer_->DebugCircles();
}

void DrawEffects() {
  if (debug::Flag("Draw Physics Circles")) {
    DrawPhysicsCircles();
  }

  if (debug::Flag("Draw Renderer Circles")) {
    DrawRendererCircles();
  }

}

} // namespace debug
