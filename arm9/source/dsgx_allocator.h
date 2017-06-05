#ifndef DSGX_ALLOCATOR_H
#define DSGX_ALLOCATOR_H

#include <map>
#include <string>

#include <nds.h>


class Dsgx;

class DsgxAllocator {
  public:
    DsgxAllocator();
    ~DsgxAllocator();
    Dsgx* Load(std::string name, const u8* data, u32 size);
    Dsgx* Retrieve(std::string name);
    void Reset();

    static const u32 kPoolSize = 1024 * 1024;
  private:
    u8* base_;
    u8* next_element_;
    u8* end_;
    std::map<std::string, Dsgx*> loaded_assets;
};  // namespace DsgxAllocator

extern u8 dsgx_pool[];

#endif  // DSGX_ALLOCATOR_H
