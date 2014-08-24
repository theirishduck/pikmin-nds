#include "DSGX.h"
#include <stdio.h>
#include <string>

//size of the chunk header, in BYTES
#define CHUNK_HEADER_SIZE 2

DSGX::DSGX(u32* data, const u32 length) {
    u32 seek = 0;
    while (seek < (length >> 2)) {
        int chunk_size = process_chunk(&data[seek]);
        seek += chunk_size;
    }
}

u32 DSGX::process_chunk(u32* location) {
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
    return chunk_length + CHUNK_HEADER_SIZE; //return the size of this chunk, for the reader to skip
}

void DSGX::dsgx_chunk(u32* data) {
    model_data = data;
}

void DSGX::bounding_sphere_chunk(u32* data) {
    center.x = (gx::Fixed<s32,12>)data[0];
    center.y = (gx::Fixed<s32,12>)data[1];
    center.z = (gx::Fixed<s32,12>)data[2];
    radius = (gx::Fixed<s32,12>)data[3];
}

void DSGX::cost_chunk(u32* data) {
    draw_cost = data[0];
}

