#ifndef VRAM_ALLOCATOR_H
#define VRAM_ALLOCATOR_H

#include <nds.h>

#include <map>
#include <string>

struct Texture {
    u16* offset;
    int width;
    int height;
    int format_width;
    int format_height;
};

class VramAllocator {
  public:
    VramAllocator(u16* cpu_base, u32 size);
    ~VramAllocator();

    Texture Load(std::string name, const u8* data, u32 size, int width, int height);
    Texture Replace(std::string name, const u8* data, u32 size);
    Texture Retrieve(std::string name);
    void Reset();

    u16* Base();
  private:
    u16* base_;
    u16 texture_offset_base_;
    u16* next_element_;
    u16* end_;

    std::map<std::string, Texture> loaded_assets;
};

#endif