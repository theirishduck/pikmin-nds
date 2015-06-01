#include "vram_allocator.h"
#include "debug.h"

using debug::nocashNumber;

/*

VramAllocator::VramAllocator(u16* base, u32 size) {
  
}

int texture_format_size(int pixel_size) {
  int format_size = 0;
  while (pixel_size > 8 and format_size < 7) {
    pixel_size = pixel_size >> 1;
    format_size++;
  }
  return format_size;
}

Texture VramAllocator::Load(std::string name, const u8* data, u32 size, int width, int height, int format) {
  
}

// Replaces a given asset with a modified version. Does NOT resize the heap;
// use this only with new assets that are the same size as the original.
Texture VramAllocator::Replace(std::string name, const u8* data, u32 size) {
  if (loaded_assets.count(name) > 0) {
    Texture destination = loaded_assets[name];
    dmaCopy(data, destination.offset, size);
    return loaded_assets[name];
  } else {
    nocashMessage("Couldn't replace; doesn't exist!");
    nocashMessage(name.c_str());
    return Texture{};
  }
}

Texture VramAllocator::Retrieve(std::string name) {
  if (loaded_assets.count(name) > 0) {
    return loaded_assets[name];
  } else {
    nocashMessage("Bad Retrieve!!");
    nocashMessage(name.c_str());
    return Texture{}; // bad things! panicing!
  }
}

u16* VramAllocator::Base() {
  return base_;
}

void VramAllocator::Reset() {
  next_element_ = base_;
}

*/