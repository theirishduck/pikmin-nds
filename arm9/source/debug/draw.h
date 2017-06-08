#ifndef DEBUG_DRAW_H
#define DEBUG_DRAW_H

class MultipassRenderer;
namespace physics { class World; }

namespace debug {

void RegisterRenderer(MultipassRenderer* renderer);
void RegisterWorld(physics::World* world);

void DrawEffects();

} // namespace debug

#endif
