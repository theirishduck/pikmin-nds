#ifndef DSGX_H
#define DSGX_H

#include <map>
#include <string>
#include <vector>

#include <nds/arm9/videoGL.h>
#include <nds/ndstypes.h>

#include "vector.h"
#include "vram_allocator.h"

struct Bone {
  char* name;
  u32 num_offsets;
  u32* offsets;
};

struct Animation {
  u32 length;  // Animation length in frames.
  m4x4* transforms;
};

struct Texture {
  char* name;
  u32 num_offsets;
  u32* offsets;
};

// Represents the contents of a .dsgx file.
// Dsgx parses .dsgx contents and provides accessors for its content.
class Dsgx {
 public:
  template <typename FixedT, int FixedF>
  using Fixed = numeric_types::Fixed<FixedT, FixedF>;

  Dsgx(u32* data, const u32 length);

  u32* DrawList();
  Vec3& Center();
  void SetCenter(Vec3 center);
  Fixed<s32, 12> Radius();

  u32 DrawCost();

  Animation* GetAnimation(std::string name);
  void ApplyAnimation(Animation* animation, u32 frame);

  void ApplyTextures(VramAllocator& texture_allocator);

private:
  u32 ProcessChunk(u32* location);
  void DsgxChunk(u32* data);
  void BoundingSphereChunk(void* data);
  void CostChunk(u32* data);
  void BoneChunk(u32* data);
  void BaniChunk(u32* data);
  void TextureChunk(u32* data);

  u32* model_data_;

  Vec3 bounding_center_;
  Fixed<s32, 12> bounding_radius_;

  u32 draw_cost_;

  std::vector<Bone> bones_;
  std::vector<Texture> textures_;
  std::map<std::string, Animation> animations_;
};

#endif
