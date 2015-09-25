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

struct TextureParam {
  char* name;
  u32 num_offsets;
  u32* offsets;
};

struct Mesh {
  template <typename FixedT, int FixedF>
  using Fixed = numeric_types::Fixed<FixedT, FixedF>;
  u32* model_data{nullptr};
  Vec3 bounding_center;
  Fixed<s32, 12> bounding_radius;
  u32 draw_cost{0};

  std::vector<Bone> bones;
  std::vector<TextureParam> textures;
};

// Represents the contents of a .dsgx file.
// Dsgx parses .dsgx contents and provides accessors for its content.
class Dsgx {
 public:
  template <typename FixedT, int FixedF>
  using Fixed = numeric_types::Fixed<FixedT, FixedF>;

  Dsgx(u32* data, const u32 length);

  Mesh* MeshByName(const char* mesh_name);
  Mesh* DefaultMesh();

  Animation* GetAnimation(std::string name);
  void ApplyAnimation(Animation* animation, u32 frame, Mesh* mesh);

  void ApplyTextures(VramAllocator<Texture>* texture_allocator, VramAllocator<TexturePalette>* palette_allocator);

private:
  u32 ProcessChunk(u32* location);
  void DsgxChunk(u32* data);
  void BoundingSphereChunk(u32* data);
  void CostChunk(u32* data);
  void BoneChunk(u32* data);
  void BaniChunk(u32* data);
  void TextureChunk(u32* data);

  std::map<std::string, Mesh> meshes_;
  std::map<std::string, Animation> animations_;
};

#endif
