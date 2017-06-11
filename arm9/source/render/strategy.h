#ifndef RENDER_STRATEGY_H
#define RENDER_STRATEGY_H

class MultipassRenderer;

namespace render {

class Strategy {
  public:
    virtual void InitializeRender(MultipassRenderer& renderer) = 0;
    virtual void DrawPartition(MultipassRenderer& renderer, int partition, bool last_partition) = 0;
};

} // namespace render

#endif
