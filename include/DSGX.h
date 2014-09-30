#ifndef DSGX_H
#define DSGX_H

#include <map>
#include <string>
#include <vector>

#include <nds/arm9/videoGL.h>
#include <nds/ndstypes.h>

#include "vector.h"

struct Bone {
  char* name;
  u32 num_offsets;
  u32* offsets;
};

struct Animation {
  u32 length;  // Animation length in frames.
  m4x4* transforms;
};

// Represents the contents of a .dsgx file.
// Dsgx parses .dsgx contents and provides accessors for its content.
class DSGX {
 public:
  DSGX(u32* data, const u32 length);

  u32* drawList();
  Vec3 center();
  void setCenter(Vec3 center);
  gx::Fixed<s32, 12> radius();

  u32 drawCost();

  Animation* getAnimation(std::string name);
  void applyAnimation(Animation* animation, u32 frame);

private:
  u32* model_data;

  Vec3 bounding_center;
  gx::Fixed<s32, 12> bounding_radius;

  u32 draw_cost;

  u32 process_chunk(u32* location);
  void dsgx_chunk(u32* data);
  void bounding_sphere_chunk(void* data);
  void cost_chunk(u32* data);
  void bone_chunk(u32* data);
  void bani_chunk(u32* data);

  std::vector<Bone> bones;
  std::map<std::string, Animation> animations;
};

#endif
