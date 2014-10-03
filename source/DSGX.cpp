#include "Dsgx.h"

#include <stdio.h>
#include <string>

//size of the chunk header, in BYTES
#define CHUNK_HEADER_SIZE 2

using namespace std;
namespace nt = numeric_types;

Dsgx::Dsgx(u32* data, const u32 length) {
    u32 seek = 0;
    //printf("length of Dsgx: %u\n", length);
    while (seek < (length >> 2)) {
        int chunk_size = process_chunk(&data[seek]);
        seek += chunk_size;
    }
}

u32 Dsgx::process_chunk(u32* location) {
    char* header = (char*)location;
    u32 chunk_length = location[1];
    u32* data = &location[2];

    if (strncmp(header, "DSGX", 4) == 0) {
        dsgx_chunk(data);
    }
    if (strncmp(header, "BSPH", 4) == 0) {
        bounding_sphere_chunk(data);
    }
    if (strncmp(header, "COST", 4) == 0) {
        cost_chunk(data);
    }
    if (strncmp(header, "BONE", 4) == 0) {
        bone_chunk(data);
    }
    if (strncmp(header, "BANI", 4) == 0) {
        bani_chunk(data);
    }
    return chunk_length + CHUNK_HEADER_SIZE; //return the size of this chunk, for the reader to skip
}

void Dsgx::dsgx_chunk(u32* data) {
    model_data_ = data;
}

void Dsgx::bounding_sphere_chunk(void* data) {
    bounding_center_.x.data_ = reinterpret_cast<s32*>(data)[0];
    bounding_center_.y.data_ = reinterpret_cast<s32*>(data)[1];
    bounding_center_.z.data_ = reinterpret_cast<s32*>(data)[2];
    bounding_radius_.data_   = reinterpret_cast<s32*>(data)[3];
}

void Dsgx::cost_chunk(u32* data) {
    draw_cost_ = data[0];
}

void Dsgx::bone_chunk(u32* data) {
    u32 num_bones = *data;
    //printf("Num bones: %u\n", num_bones);
    //while(true){}
    data++;
    for (u32 i = 0; i < num_bones; i++) {
        Bone bone;

        bone.name = (char*)data;
        data += 8; //skip past the string

        bone.num_offsets = *data;
        data++;

        //printf("bone offsets: %u\n", bone.num_offsets);

        bone.offsets = data;
        data += bone.num_offsets;

        bones_.push_back(bone);
    }
    //while(true){}
}

//Baked Animation
void Dsgx::bani_chunk(u32* data) {
    Animation new_anim;
    char* name = (char*)data;
    data += 8;

    new_anim.length = *data;
    data++;

    new_anim.transforms = (m4x4*)data;
    animations_[name] = new_anim;
}

u32* Dsgx::drawList() {
    return model_data_;
}

Vec3 Dsgx::center() {
    return bounding_center_;
}

void Dsgx::setCenter(Vec3 center) {
    bounding_center_ = center;
}

nt::Fixed<s32, 12> Dsgx::radius() {
    return bounding_radius_;
}

u32 Dsgx::drawCost() {
    return draw_cost_;
}

Animation* Dsgx::getAnimation(string name) {
    if (animations_.count(name) == 0) {
        printf("Couldn't find animation: %s", name.c_str());
        return 0; //bail, animation doesn't exist
    }

    return &animations_[name];
}

void Dsgx::applyAnimation(Animation* animation, u32 frame) {
    m4x4 const* current_matrix = animation->transforms + bones_.size() * frame;
    for (auto bone = bones_.begin(); bone != bones_.end(); bone++) {
        for (u32 i = 0; i < bone->num_offsets; i++) {
            *((m4x4*)(model_data_ + bone->offsets[i] + 1)) = *current_matrix;
            //try DMA copy!
            //dmaCopyWordsAsynch(0, current_matrix, (model_data_ + bone->offsets[i] + 1), sizeof(m4x4));
        }
        current_matrix++;
    }
}