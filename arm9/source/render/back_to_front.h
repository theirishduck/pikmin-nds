#ifndef RENDER_BACK_TO_FRONT_H
#define RENDER_BACK_TO_FRONT_H

#include "render/strategy.h"

namespace render {

class BackToFront : public Strategy {
  public:
    void InitializeRender(MultipassRenderer& renderer);
    void DrawPartition(MultipassRenderer& renderer, int partition, bool last_partition);
  private:
    void GatherDrawList(MultipassRenderer& renderer);
};

} // namespace render

#endif
