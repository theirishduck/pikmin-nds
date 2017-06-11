#include "render/back_to_front.h"

#include "render/multipass_renderer.h"
#include "drawable.h"
#include "numeric_types.h"

using numeric_types::literals::operator"" _f;
using numeric_types::fixed;

using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;

namespace render {

void BackToFront::GatherDrawList(MultipassRenderer& renderer) {
  // Set the projection matrix to a full frustrum so that the list can be sorted
  // without having to accout for errors caused by the clip plane.
  renderer.ClipFriendlyPerspective(0.1_f, 256.0_f, renderer.cached_camera_fov_);

  // Shift into camera space for the following tests
  glLoadIdentity();
  renderer.ApplyCameraTransform();

  for (auto entity : renderer.entities_) {
    // Cache the object so its render information stays the same across
    // multiple passes.
    entity->SetCache();
    DrawState& state = entity->GetCachedState();

    if (entity->InsideViewFrustrum()) {
      // Using the camera state, calculate the nearest and farthest points,
      // which we'll later use to decide where the clipping planes should go.
      EntityContainer container;
      container.entity = entity;
      fixed object_z = entity->GetRealModelZ();
      if (entity->important) {
        container.far_z  = object_z + state.current_mesh->bounding_radius;
        container.near_z = object_z - state.current_mesh->bounding_radius;
      } else {
        container.far_z  = object_z;
        container.near_z = object_z;
      }

      entity->visible = true;
      entity->overlaps = 0;

      renderer.draw_list_.push(container);
    } else {
      entity->visible = false;
    }
  }
}

void BackToFront::InitializeRender(MultipassRenderer& renderer) {
  GatherDrawList(renderer);
}

void BackToFront::DrawPartition(MultipassRenderer& renderer, int partition, bool last_partition) {

}

} // namespace render
