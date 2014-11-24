#include "pellet_posy.h"

#include "dsgx.h"

//Model Data
#include "pellet_posy_dsgx.h"

using entities::PelletPosy;

using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;

PelletPosy::PelletPosy(VramAllocator& texture_allocator) {
  // Todo(Nick) Share Dsgx instances across instances.
  Dsgx* posy_actor = new Dsgx((u32*)pellet_posy_dsgx, pellet_posy_dsgx_size);
  posy_actor->ApplyTextures(texture_allocator);
  set_actor(posy_actor);
  SetAnimation("Armature|Idle");
  set_rotation(0_brad, 0_brad, 0_brad);
}

PelletPosy::~PelletPosy() {
  delete actor();
}