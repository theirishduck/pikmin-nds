#include "dsgx.h"

#include <cstdio>

#include <string>
#include "debug.h"

using namespace std;
namespace nt = numeric_types;

constexpr u32 kChunkHeaderSizeWords{2};

Dsgx::Dsgx(u32* data, const u32 length) {
  u32 seek = 0;
  // printf("length of Dsgx: %u\n", length);
  while (seek < (length >> 2)) {
    int const chunk_size = ProcessChunk(&data[seek]);
    seek += chunk_size;
  }
}

u32 Dsgx::ProcessChunk(u32* location) {
  char* header = (char*)location;
  u32 chunk_length = location[1];
  u32* data = &location[2];

  if (strncmp(header, "DSGX", 4) == 0) {
    DsgxChunk(data);
  }
  if (strncmp(header, "BSPH", 4) == 0) {
    BoundingSphereChunk(data);
  }
  if (strncmp(header, "COST", 4) == 0) {
    CostChunk(data);
  }
  if (strncmp(header, "BONE", 4) == 0) {
    BoneChunk(data);
  }
  if (strncmp(header, "BANI", 4) == 0) {
    BaniChunk(data);
  }
  if (strncmp(header, "TXTR", 4) == 0) {
    TextureChunk(data);
  }
  // Return the size of this chunk so the reader can skip to the next chunk.
  return chunk_length + kChunkHeaderSizeWords;
}

void Dsgx::DsgxChunk(u32* data) {
  model_data_ = data;
}

void Dsgx::BoundingSphereChunk(void* data) {
  bounding_center_.x.data_ = reinterpret_cast<s32*>(data)[0];
  bounding_center_.y.data_ = reinterpret_cast<s32*>(data)[1];
  bounding_center_.z.data_ = reinterpret_cast<s32*>(data)[2];
  bounding_radius_.data_   = reinterpret_cast<s32*>(data)[3];
}

void Dsgx::CostChunk(u32* data) {
  draw_cost_ = data[0];
}

void Dsgx::BoneChunk(u32* data) {
  u32 num_bones = *data;
  // printf("Num bones: %u\n", num_bones);
  // while(true){}
  data++;
  for (u32 i = 0; i < num_bones; i++) {
    Bone bone;
    bone.name = (char*)data;
    data += 8;  // Skip past the bone name.

    bone.num_offsets = *data;
    data++;

    // printf("bone offsets: %u\n", bone.num_offsets);

    bone.offsets = data;
    data += bone.num_offsets;

    bones_.push_back(bone);
  }
  // while(true){}
}

// BANI is short for Baked ANImation.
void Dsgx::BaniChunk(u32* data) {
  Animation new_anim;
  char* name = (char*)data;
  data += 8;

  new_anim.length = *data;
  data++;

  new_anim.transforms = (m4x4*)data;
  animations_[name] = new_anim;
}

void Dsgx::TextureChunk(u32* data) {
  u32 num_textures = *data;
  data++;
  nocashMessage("Loading Textures...");

  for (u32 i = 0; i < num_textures; i++) {
    Texture texture;
    texture.name = (char*)data;
    data += 8; //skip past the texture name

    texture.num_offsets = *data;
    data++;

    texture.offsets = data;
    data += texture.num_offsets;

    textures_.push_back(texture);

    nocashMessage(texture.name);
  }
}

u32* Dsgx::DrawList() {
  return model_data_;
}

Vec3& Dsgx::Center() {
  return bounding_center_;
}

void Dsgx::SetCenter(Vec3 center) {
  bounding_center_ = center;
}

nt::Fixed<s32, 12> Dsgx::Radius() {
  return bounding_radius_;
}

u32 Dsgx::DrawCost() {
  return draw_cost_;
}

Animation* Dsgx::GetAnimation(string name) {
  if (animations_.count(name) == 0) {
    printf("Couldn't find animation: %s", name.c_str());
    return nullptr;  // The requested animation doesn't exist.
  }

  return &animations_[name];
}

void Dsgx::ApplyAnimation(Animation* animation, u32 frame) {
  auto destination = model_data_ + 1;
  m4x4 const* current_matrix = animation->transforms + bones_.size() * frame;
  for (auto bone = bones_.begin(); bone != bones_.end(); bone++) {
    for (u32 i = 0; i < bone->num_offsets; i++) {
      *((m4x4*)(destination + bone->offsets[i])) = *current_matrix;
    }
    current_matrix++;
  }
}

void Dsgx::ApplyTextures(VramAllocator& texture_allocator) {
  // go through this object's textures and write in the correct offsets
  // into VRAM, based on where they got loaded
  auto destination = model_data_ + 1;
  for (auto texture = textures_.begin(); texture != textures_.end(); texture++) {
    u32 location = (u32)texture_allocator.Retrieve(texture->name) - (u32)texture_allocator.Base();
    for (u32 i = 0; i < texture->num_offsets; i++) {
      *(destination + texture->offsets[i]) = ((*(destination + texture->offsets[i]))) + (location / 8);
      nocashMessage("Wrote an offset!");
      debug::nocashNumber((int)location);
      debug::nocashNumber(((*(destination + texture->offsets[i]))) + (location / 8));
      
    }
  }
}
