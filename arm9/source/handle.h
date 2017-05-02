#ifndef HANDLE_H
#define HANDLE_H

// A container and handle pair, designed to add safety to a naive fixed-memory
// allocation routine that may place new objects into old pointer slots.

struct Handle {
public:
  unsigned int id;
  unsigned int generation;
  unsigned int type;
};

template <typename ObjectType>
struct HandleWrapper {
public:
  Handle handle;
  ObjectType object;

  bool IsHandleValid(Handle other) {
    return handle.id == other.id and handle.generation == other.generation and handle.type == other.type;
  }
};

#endif
