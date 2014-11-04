#ifndef VRAM_ALLOCATOR_H
#define VRAM_ALLOCATOR_H

#include <nds.h>

#include <map>
#include <string>

class VramAllocator {
  public:
    VramAllocator(u16* base, u32 size);
    ~VramAllocator();

    u16* Load(std::string name, const u8* data, u32 size);
    u16* Retrieve(std::string name);

    u16* Base();

  private:
    u16* base_;
    u16* next_element_;
    u16* end_;

    std::map<std::string, u16*> loaded_assets;
};

#endif