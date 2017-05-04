#ifndef HANDLE_H
#define HANDLE_H

// A container and handle pair, designed to add safety to a naive fixed-memory
// allocation routine that may place new objects into old pointer slots.

struct Handle {
public:
  unsigned int id{0};
  unsigned int generation{0};
  unsigned int type{0};
  bool Matches(Handle other) {
    return id == other.id and generation == other.generation and type == other.type;
  }
};

#endif
