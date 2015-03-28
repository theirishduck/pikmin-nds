#include "level.h"

#include "dsgx.h"

//Model Data
#include "sandbox_test_dsgx.h"
#include "checkerboard_test_dsgx.h"

using entities::Level;

using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;

Level::Level(VramAllocator& texture_allocator) {
  // Todo(Nick) Share Dsgx instances across instances.
  Dsgx* level_actor = new Dsgx((u32*)checkerboard_test_dsgx, checkerboard_test_dsgx_size);
  level_actor->ApplyTextures(texture_allocator);
  set_actor(level_actor);
  set_rotation(0_brad, 0_brad, 0_brad);
}

Level::~Level() {
  delete actor();
}

void Level::Init() {
  DrawableEntity::Init();
  body_->collides_with_level = 0;
  body_->affected_by_gravity = 0;
}