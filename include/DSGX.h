#ifndef DSGX_H
#define DSGX_H

#include <map>
#include <string>
#include <vector>

#include <nds.h>

#include "vector.h"

// class to represent a .dsgx file. Contains all logic necessary to read in and
// decode .dsgx contents, and provides methods for accessing that content in a
// straightforward manner. Note that this class does NOT perform any type conversions;
// make sure you have a system in place for handling FixedPoint data.

struct Bone {
    char* name;
    u32 num_offsets;
    u32* offsets;
};

struct Animation {
    u32 length; //in frames
    m4x4* transforms;
};

class DSGX {
    private:
        //put stuff here
        u32* model_data;

        Vec3 bounding_center;
        gx::Fixed<s32,12> bounding_radius;

        u32 draw_cost;

        u32 process_chunk(u32* location);
        void dsgx_chunk(u32* data);
        void bounding_sphere_chunk(void* data);
        void cost_chunk(u32* data);
        void bone_chunk(u32* data);
        void bani_chunk(u32* data);

        std::vector<Bone> bones;
        std::map<std::string, Animation> animations;
    public:
        DSGX(u32* data, const u32 length);

        u32* drawList();
        Vec3 center();
        void setCenter(Vec3 center);
        gx::Fixed<s32,12> radius();

        u32 drawCost();

        Animation* getAnimation(std::string name);
        void applyAnimation(Animation* animation, u32 frame);

};

#endif