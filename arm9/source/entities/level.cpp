#include "level.h"

#include "dsgx.h"

using entities::Level;

using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;

Level::Level(VramAllocator<Texture>* texture_allocator, VramAllocator<TexturePalette>* palette_allocator) {
  set_rotation(0_brad, 0_brad, 0_brad);
}

Level::~Level() {
  //delete actor();
}

void Level::Init() {
  DrawableEntity::Init();
  body_->collides_with_level = 0;
  body_->affected_by_gravity = 0;
}
