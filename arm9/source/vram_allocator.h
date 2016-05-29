#ifndef VRAM_ALLOCATOR_H
#define VRAM_ALLOCATOR_H

#include <nds.h>

#include <map>
#include <string>
#include <algorithm>
#include "debug.h"

struct Texture {
  int format_width;
  int format_height;
  int format;
  enum {
    kDisplayed = 0,
    kTransparent = 1
  } transparency;
  u16* offset;
};

struct TexturePalette {
  int colors;
  u16* offset;
};

struct Sprite {
  int pixel_width;
  int pixel_height;
  u16* offset;
};

template<typename T>
class VramAllocator {
  private:
    u16* base_;
    u16 texture_offset_base_;
    u16* next_element_;
    u16* end_;
    u32 alignment_;

    std::map<std::string, T> loaded_assets;

  public:
    using Metadata = T;

    VramAllocator(u16* cpu_base, u32 size, u32 alignment = 1) {
      this->base_ = cpu_base;
      this->end_ = cpu_base + size / sizeof(u16);
      this->next_element_ = cpu_base;
      this->alignment_ = alignment;
      //nocashMessage("Constructor called with size: ");
      //debug::nocashNumber(size);
    }
    ~VramAllocator() {}

    Metadata Load(std::string name, const u8* data, u32 size, Metadata metadata) {
      if (loaded_assets.count(name) > 0) {
        //nocashMessage("Already loaded!");
        // this is already loaded! Just return a reference to the data
        return loaded_assets[name];
      }

      if (next_element_ + size / sizeof(u16) > end_) {
        nocashMessage("Not enough room for:");
        nocashMessage(name.c_str());
        nocashMessage("next element was:");
        debug::nocashNumber((int)next_element_);
        nocashMessage("size was:");
        debug::nocashNumber((int)size);
        nocashMessage("end was:");
        debug::nocashNumber((int)end_);
        return T{}; // we don't have enough room for this object! and there was
                  // panic. much panic.
      }

      u16* destination = next_element_;
      // copy the data into VRAM
      for (unsigned int i = 0; i < size / sizeof(u16); i++) {
        destination[i] = ((u16*)data)[i];
      }

      // offset the next element for the next call to Load
      next_element_ += std::max(size / sizeof(u16), alignment_ / sizeof(u16));

      loaded_assets[name] = metadata;
      loaded_assets[name].offset = destination;

      //nocashMessage("Loaded Texture: ");
      //nocashMessage(name.c_str());
      //debug::nocashNumber(destination - base_);

      // return the address we just copied data to, for immediate use
      return loaded_assets[name];
    }

    T Replace(std::string name, const u8* data, u32 size) {
      if (loaded_assets.count(name) > 0) {
        T destination = loaded_assets[name];
        dmaCopy(data, destination.offset, size);
        return loaded_assets[name];
      } else {
        nocashMessage("Couldn't replace; doesn't exist!");
        nocashMessage(name.c_str());
        return T{};
      }
    }
    T Retrieve(std::string name) {
      if (loaded_assets.count(name) > 0) {
        return loaded_assets[name];
      } else {
        nocashMessage("Bad Retrieve!!");
        nocashMessage(name.c_str());
        return T{}; // bad things! panicing!
      }
    }
    void Reset() {
      next_element_ = base_;
      loaded_assets.clear();
    }

    u16* Base() {
      return base_;
    }
};

#endif
