#ifndef DSGX_H
#define DSGX_H

#include <nds.h>
#include "vector.h"

// class to represent a .dsgx file. Contains all logic necessary to read in and
// decode .dsgx contents, and provides methods for accessing that content in a
// straightforward manner. Note that this class does NOT perform any type conversions;
// make sure you have a system in place for handling FixedPoint data.

class DSGX {
    private:
        //put stuff here
        u32* model_data;

        Vec3 bounding_center;
        gx::Fixed<s32,12> bounding_radius;

        u32 draw_cost;

        u32 process_chunk(u32* location);
        void dsgx_chunk(u32* data);
        void bounding_sphere_chunk(u32* data);
        void cost_chunk(u32* data);
    public:
        DSGX(u32* data, const u32 length);

        u32* drawList();
        Vec3 center();
        gx::Fixed<s32,12> radius();

        u32 drawCost();

};

#endif