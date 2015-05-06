#include "vram_allocator.h"
#include "debug.h"

using debug::nocashNumber;

VramAllocator::VramAllocator(u16* base, u32 size) {
  this->base_ = base;
  this->end_ = base + size / sizeof(u16);
  this->next_element_ = base;
  nocashMessage("Constructor called with size: ");
  nocashNumber(size);
}

VramAllocator::~VramAllocator() {
}

int texture_format_size(int pixel_size) {
  int format_size = 0;
  while (pixel_size > 8 and format_size < 7) {
    pixel_size = pixel_size >> 1;
    format_size++;
  }
  return format_size;
}

Texture VramAllocator::Load(std::string name, const u8* data, u32 size, int width, int height) {
  if (loaded_assets.count(name) > 0) {
    nocashMessage("Already loaded!");
    // this is already loaded! Just return a reference to the data
    return loaded_assets[name];
  }

  if (next_element_ + size / sizeof(u16) > end_) {
    nocashMessage("Not enough room for:");
    nocashMessage(name.c_str());
    nocashMessage("next element was:");
    nocashNumber((int)next_element_);
    nocashMessage("size was:");
    nocashNumber((int)size);
    nocashMessage("end was:");
    nocashNumber((int)end_);
    return Texture{}; // we don't have enough room for this object! and there was
              // panic. much panic.
  }

  u16* destination = next_element_;
  // copy the data into VRAM
  dmaCopy(data, destination, size);

  // offset the next element for the next call to Load
  next_element_ += size / sizeof(u16);


  loaded_assets[name] = Texture{destination, width, height, texture_format_size(width), texture_format_size(height)};

  nocashMessage("Loaded Texture: ");
  nocashMessage(name.c_str());

  // return the address we just copied data to, for immediate use
  return loaded_assets[name];
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