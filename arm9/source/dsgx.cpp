#include "dsgx.h"

#include <cstdio>

#include <string>
#include "debug/messages.h"
#include "debug/utilities.h"

using namespace std;
namespace nt = numeric_types;
using nt::literals::operator"" _f;

constexpr u32 kChunkHeaderSizeWords{2};

Dsgx::Dsgx(u32* data, const u32 length):
    meshes_{},
    bone_animations_{} {
  u32 seek = 0;
  while (seek < (length >> 2)) {
    int const chunk_size = ProcessChunk(&data[seek]);
    seek += chunk_size;
  }

  // Nice-ify the animation data
  CollectAnimations();

  // Print out a crapton of debug info
  //debug::Log("== DSGX Data ==");
  //debug::Log("Number of meshes" + debug::to_string(meshes_.size()));
  //for (auto mesh : meshes_) {
  //  debug::Log("-- Mesh: " + mesh.first + " --");
  //}
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
  if (strncmp(header, "AREF", 4) == 0) {
    ArefChunk(data);
  }
  if (strncmp(header, "ANIM", 4) == 0) {
    AnimChunk(data);
  }

  // Return the size of this chunk so the reader can skip to the next chunk.
  return chunk_length + kChunkHeaderSizeWords;
}

void Mesh::AddAnimation(char* name, u32 length, AnimationReference ref, AnimationData data) {
  animations.emplace(name, Animation());
  animations[name].name = name;
  animations[name].frame_length = length;
  animations[name].channels.push_back(std::make_pair(ref, data));
}

void Dsgx::CollectAnimations() {
  // Run through the animation data that we read in, and store that data in the
  // mesh for easier access.
  for (auto anim : animation_data_) {
    // First, we need to see if there's a matching animation reference for this
    // mesh / type
    bool found_reference = false;
    for (auto aref : animation_references_) {
      if (strcmp(aref.data_type, anim.data_type) == 0) {
        if (strlen(anim.mesh_name) == 0 or strcmp(aref.mesh_name, anim.mesh_name) == 0) {
          // Now go through and pair this anim/aref with every mesh that matches it
          for (auto kv : meshes_) {
            if (strcmp(aref.mesh_name, kv.first.c_str()) == 0) {
              kv.second.AddAnimation(anim.animation_name, anim.frame_length,
                aref, anim);
              meshes_[kv.first] = kv.second;
              found_reference = true;
              //debug::Log("Added ANIM" + anim.animation_name);
              //debug::Log("To Mesh   " + anim.mesh_name);
            }
          }
        }
      }
    }
    if (!found_reference) {
        debug::Log("No AREF found for ANIM: " + std::string(anim.animation_name) + std::string(anim.data_type));
        //debug::Log(anim.animation_name);
        //debug::Log(anim.data_type);
    }
  }
}

void Dsgx::DsgxChunk(u32* data) {
  char* mesh_name = (char*)data;
  meshes_.emplace(mesh_name, Mesh());
  data += 8;  // Skip past the name
  meshes_[mesh_name].name = mesh_name;
  meshes_[mesh_name].model_data = data;
}

void Dsgx::BoundingSphereChunk(u32* data) {
  char* mesh_name = (char*)data;
  meshes_.emplace(mesh_name, Mesh());
  data += 8;  // Skip past the name

  meshes_[mesh_name].bounding_center.x.data_ = reinterpret_cast<s32*>(data)[0];
  meshes_[mesh_name].bounding_center.y.data_ = reinterpret_cast<s32*>(data)[1];
  meshes_[mesh_name].bounding_center.z.data_ = reinterpret_cast<s32*>(data)[2];
  meshes_[mesh_name].bounding_radius.data_   = reinterpret_cast<s32*>(data)[3];
}

void Dsgx::CostChunk(u32* data) {
  char* mesh_name = (char*)data;
  meshes_.emplace(mesh_name, Mesh());
  data += 8;  // Skip past the name

  meshes_[mesh_name].draw_cost = data[0];
}

void Dsgx::BoneChunk(u32* data) {
  char* mesh_name = (char*)data;
  meshes_.emplace(mesh_name, Mesh());
  data += 8;  // Skip past the name

  u32 num_bones = *data;
  data++;
  for (u32 i = 0; i < num_bones; i++) {
    BoneReference bone;
    bone.name = (char*)data;
    data += 8;  // Skip past the bone name.

    bone.num_offsets = *data;
    data++;

    //debug::Log(bone.name + ": " + debug::to_string(bone.num_offsets) + " offsets");

    bone.offsets = data;
    data += bone.num_offsets;

    meshes_[mesh_name].bones.push_back(bone);
  }
}

// BANI is short for Baked ANImation.
void Dsgx::BaniChunk(u32* data) {
  BoneAnimation new_anim;
  char* name = (char*)data;
  data += 8;

  new_anim.length = *data;
  data++;

  new_anim.transforms = (m4x4*)data;
  bone_animations_[name] = new_anim;
}

void Dsgx::TextureChunk(u32* data) {
  char* mesh_name = (char*)data;
  meshes_.emplace(mesh_name, Mesh());
  data += 8;  // Skip past the name

  u32 num_textures = *data;
  data++;
  //debug::Log("Loading Textures...");

  for (u32 i = 0; i < num_textures; i++) {
    TextureParam texture;
    texture.name = (char*)data;
    data += 8; //skip past the texture name

    texture.num_offsets = *data;
    data++;

    texture.offsets = data;
    data += texture.num_offsets;

    meshes_[mesh_name].textures.push_back(texture);

    //debug::Log(texture.name);
  }
}

void Dsgx::ArefChunk(u32* data) {
  AnimationReference aref;
  aref.data_type = (char*) data;
  data += 8;
  aref.mesh_name = (char*) data;
  data += 8;

  aref.num_references = *data;
  data++;

  for (u32 i = 0; i < aref.num_references; i++) {
    OffsetList offset_list;
    offset_list.name = (char*)data;
    data += 8;  // Skip past the reference name.

    offset_list.num_offsets = *data;
    data++;

    offset_list.offsets = data;
    data += offset_list.num_offsets;

    aref.offset_lists.push_back(offset_list);
  }

  animation_references_.push_back(aref);

  debug::Log("Loaded AREF: ");
  debug::Log(aref.data_type);
  debug::Log(aref.mesh_name);
}

void Dsgx::AnimChunk(u32* data) {
  AnimationData anim;
  anim.animation_name = (char*) data;
  data += 8;
  anim.data_type = (char*) data;
  data += 8;
  anim.mesh_name = (char*) data;
  data += 8;

  anim.frame_length = *data;
  data++;
  anim.word_count = *data;
  data++;
  anim.data = data;

  animation_data_.push_back(anim);

  //debug::nocashValue("Loaded ANIM", anim.animation_name);
  //debug::nocashValue("Type", anim.data_type);
  //debug::nocashValue("Mesh", anim.mesh_name);
  //debug::nocashValue("Length", anim.frame_length);
  //debug::nocashValue("Word Count", anim.word_count);
}

Mesh* Dsgx::MeshByName(const char* mesh_name) {
  if (meshes_.count(mesh_name) > 0) {
    return &meshes_[mesh_name];
  }
  return nullptr;
}

Mesh* Dsgx::DefaultMesh() {
  return &meshes_.begin()->second;
}

Animation* Dsgx::GetAnimation(string name, Mesh* mesh) {
  if (mesh->animations.count(name) == 0) {
    debug::Log("Could not load ANIM: " + name);
    debug::Log("From mesh: " + std::string(mesh->name));
    debug::Log("With total anims: " + debug::to_string(mesh->animations.size()));
    return nullptr;  // The requested animation doesn't exist.
  }
  //debug::nocashValue("Switched to ANIM", name);
  //debug::nocashValue("Length", mesh->animations[name].frame_length);
  //debug::nocashValue("Channels", mesh->animations[name].channels.size());
  //debug::nocashValue("Data Size", mesh->animations[name].channels[0].first.num_references);
  return &mesh->animations[name];
}

void Dsgx::ApplyAnimation(Animation* animation, u32 frame, Mesh* mesh) {
  auto destination = mesh->model_data + 1;
  for (auto& channel : animation->channels) {
    auto& ref = channel.first;
    auto& data = channel.second;
    u32 const* current_data = data.data;
    current_data += ref.num_references * data.word_count * frame;
    if (data.word_count == 1) {
      //Optimized Case for 1-word copies, avoids the inner loop
      for (auto offset_list = ref.offset_lists.begin(); offset_list != ref.offset_lists.end(); offset_list++) {
        for (u32 i = 0; i < offset_list->num_offsets; i++) {
            *((u32*)(destination + offset_list->offsets[i])) = *current_data;
        }
        current_data++;
      }
    } else {
      for (auto offset_list = ref.offset_lists.begin(); offset_list != ref.offset_lists.end(); offset_list++) {
        for (u32 i = 0; i < offset_list->num_offsets; i++) {
          for (u32 d = 0; d < data.word_count; d++) {
            *((u32*)(destination + offset_list->offsets[i] + d)) = current_data[d];
          }
        }
        current_data += data.word_count;
      }
    }
  }
}

BoneAnimation* Dsgx::GetBoneAnimation(string name) {
  if (bone_animations_.count(name) == 0) {
    debug::Log("Couldn't find bone animation: " + name);
    return nullptr;  // The requested animation doesn't exist.
  }

  return &bone_animations_[name];
}

void Dsgx::ApplyBoneAnimation(BoneAnimation* animation, u32 frame, Mesh* mesh) {
  auto destination = mesh->model_data + 1;
  m4x4 const* current_matrix = animation->transforms + mesh->bones.size() * frame;
  for (auto bone = mesh->bones.begin(); bone != mesh->bones.end(); bone++) {
    for (u32 i = 0; i < bone->num_offsets; i++) {
      *((m4x4*)(destination + bone->offsets[i])) = *current_matrix;
    }
    current_matrix++;
  }
}

void Dsgx::ApplyTextures(VramAllocator<Texture>* texture_allocator, VramAllocator<TexturePalette>* palette_allocator) {
  for (auto& m : meshes_) {
    Mesh* mesh = &m.second;
    // go through this object's textures and write in the correct offsets
    // into VRAM, based on where they got loaded
    auto destination = mesh->model_data + 1;
    for (auto texture = mesh->textures.begin(); texture != mesh->textures.end(); texture++) {
      auto loaded_texture = texture_allocator->Retrieve(texture->name);
      // First, write the offset to actual TEXEL data; we always need to do this
      u32 location = (u32)loaded_texture.offset;
      location /= 8;
      for (u32 i = 0; i < texture->num_offsets; i++) {
        //set the texture offset
        u32 const kTextureOffsetMask = 0x0000FFFF;
        destination[texture->offsets[i]] =
          (destination[texture->offsets[i]] & ~kTextureOffsetMask) | (location & kTextureOffsetMask);

        //set the format (warning: funky hex binary logic here)
        u32 const kTextureFormatMask = 0x3C000000;
        destination[texture->offsets[i]] =
          (destination[texture->offsets[i]] & ~kTextureFormatMask) |
          ((loaded_texture.format << 26 | loaded_texture.transparency << 29) & kTextureFormatMask);
      }
      // If this is any texture format other than Direct Texture, then we need to
      // also write in the PALETTE BASE data; this is a little funky
      if (loaded_texture.format != GL_RGBA) {
        auto loaded_palette = palette_allocator->Retrieve(texture->name);
        u32 palette_location = (u32)loaded_palette.offset - (u32)palette_allocator->Base();
        // if this is a 4bpp texture (format 2) we use 8-byte offsets
        // otherwise we use 16 byte offsets
        if (loaded_texture.format == GL_RGB4) {
          palette_location /= 8;
        } else {
          palette_location /= 16;
        }
        for (u32 i = 0; i < texture->num_offsets; i++) {
          // The +2 skips command and parameters; PLTT_BASE is always stored
          // immediately after TEXIMAGE_PARAM, and we don't use packed commands.
          destination[texture->offsets[i] + 2] = palette_location;
        }
      }
    }
  }
}
