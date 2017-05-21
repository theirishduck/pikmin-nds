#include "dsgx.h"
#include "dsgx_allocator.h"
#include "debug/messages.h"
#include "debug/utilities.h"

#include <string>

u8 dsgx_pool[DsgxAllocator::kPoolSize];  // 1 MB

using std::string;

DsgxAllocator::DsgxAllocator() {
  base_ = dsgx_pool;
  next_element_ = base_;
  end_ = base_ + kPoolSize;
}

DsgxAllocator::~DsgxAllocator() {
}

Dsgx* DsgxAllocator::Load(string name, const u8* data, u32 size) {
  if (loaded_assets.count(name) > 0) {
    debug::Log("Already loaded!");
    // this is already loaded! Just return a reference to the data
    return loaded_assets[name];
  }

  if (next_element_ + size / sizeof(u8) > end_) {
    debug::Log("Not enough room for:");
    debug::Log(name.c_str());
    debug::Log("next element was: " + debug::to_string((int)next_element_));
    //debug::nocashNumber((int)next_element_);
    debug::Log("size was: " + debug::to_string((int)size));
    //debug::nocashNumber((int)size);
    debug::Log("end was: " + debug::to_string((int)end_));
    //debug::nocashNumber((int)end_);
    return nullptr; // we don't have enough room for this object! and there was
              // panic. much panic.
  }

  u8* destination = next_element_;
  // copy the data into the pool
  for (unsigned int i = 0; i < size / sizeof(u8); i++) {
    destination[i] = ((u8*)data)[i];
  }

  // offset the next element for the next call to Load
  next_element_ += size / sizeof(u8);

  Dsgx* dsgx = new Dsgx((u32*)destination, size);
  loaded_assets[name] = dsgx;

  // return the address we just copied data to, for immediate use
  return loaded_assets[name];
}

Dsgx* DsgxAllocator::Retrieve(std::string name) {
  if (loaded_assets.count(name) > 0) {
    return loaded_assets[name];
  } else {
    debug::Log(("Bad Retrieve! - " + name).c_str());
    return nullptr; // bad things! panicing!
  }
}

void DsgxAllocator::Reset() {
  next_element_ = base_;
  for (auto asset : loaded_assets) {
    delete asset.second;
  }
  loaded_assets.clear();
}
