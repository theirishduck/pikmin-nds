#ifndef DEBUG_DRAW_H
#define DEBUG_DRAW_H

namespace physics { class World; }

namespace debug {

void RegisterWorld(physics::World* world);
void DrawEffects();

} // namespace debug

#endif
