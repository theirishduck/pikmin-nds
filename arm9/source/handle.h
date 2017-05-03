#ifndef HANDLE_H
#define HANDLE_H

// A container and handle pair, designed to add safety to a naive fixed-memory
// allocation routine that may place new objects into old pointer slots.

struct Handle {
public:
  unsigned int id;
  unsigned int generation;
  unsigned int type;
  bool IsHandleValid(Handle other) {
    return id == other.id and generation == other.generation and type == other.type;
  }
};

#endif
