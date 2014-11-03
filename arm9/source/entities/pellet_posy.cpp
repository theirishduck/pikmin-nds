#include "pellet_posy.h"

#include "dsgx.h"

//Model Data
#include "pellet_posy_dsgx.h"

using entities::PelletPosy;

using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;

PelletPosy::PelletPosy() {
  // Todo(Nick) Share Dsgx instances across Captain instances.
  Dsgx* posy_actor = new Dsgx((u32*)pellet_posy_dsgx, pellet_posy_dsgx_size);
  set_actor(posy_actor);
  set_rotation(-90_brad, 0_brad, 0_brad);
}

PelletPosy::~PelletPosy() {
  delete actor();
}