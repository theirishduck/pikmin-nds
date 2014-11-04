#include "vram_allocator.h"

VramAllocator::VramAllocator(u16* base, u32 size) {
  this->base_ = base;
  this->end_ = base + size / sizeof(16);
  this->next_element_ = base;
}

VramAllocator::~VramAllocator() {
}

u16* VramAllocator::Load(std::string name, const u8* data, u32 size) {
  if (loaded_assets.count(name) > 0) {
    nocashMessage("Already loaded!");
    // this is already loaded! Just return a reference to the data
    return loaded_assets[name];
  }

  if (next_element_ + size > end_) {
    nocashMessage("Not enough room!");
    return 0; // we don't have enough room for this object! and there was
              // panic. much panic.
  }

  u16* destination = next_element_;
  // copy the data into VRAM
  dmaCopy(data, destination, size);

  // offset the next element for the next call to Load
  next_element_ += size / sizeof(u16);

  loaded_assets[name] = destination;

  nocashMessage("Loaded Texture: ");
  nocashMessage(name.c_str());

  // return the address we just copied data to, for immediate use
  return destination;
}

u16* VramAllocator::Retrieve(std::string name) {
  if (loaded_assets.count(name) > 0) {
    return loaded_assets[name];
  } else {
    nocashMessage("Bad Retrieve!!");
    return 0; // bad things! panicing!
  }
}

u16* VramAllocator::Base() {
  return base_;
}