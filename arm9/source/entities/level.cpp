#include "level.h"

#include "dsgx.h"

using entities::Level;

using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;

Level::Level(physics::World& world) : DrawableEntity(world) {
  body_.body->collides_with_level = 0;
  body_.body->affected_by_gravity = 0;
  set_rotation(0_brad, 0_brad, 0_brad);
}
