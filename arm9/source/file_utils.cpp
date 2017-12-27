#include "file_utils.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include "debug/messages.h"
#include "debug/utilities.h"

using namespace std;

vector<string> FilesInDirectory(string path) {
  auto dir = opendir(path.c_str());
  vector<string> directories;
  if (dir) {
    while (auto file_entry = readdir(dir)) {
      if (strcmp(".", file_entry->d_name) == 0 or strcmp("..", file_entry->d_name) == 0) {
        continue;
      }
      if (file_entry->d_type == DT_DIR) {
        continue;
      }
      directories.push_back(file_entry->d_name);
    }
  }
  return directories;
}

// Note: this returns a vector which is less than ideal (heap) and most usage tosses the file
// straight into a pre-allocated buffer. Perhaps we could skip the vector and load the file directly
// into a provided pointer?
vector<char> LoadEntireFile(string filename) {
  auto file = fopen(filename.c_str(), "rb");
  if (file) {
    fseek(file, 0, SEEK_END);
    auto const size = ftell(file);
    fseek(file, 0, SEEK_SET);

    vector<char> buffer(size);
    if (fread(buffer.data(), 1, size, file)) {
      return buffer;
    } else {
      debug::Log("NitroFS Read FAILED for " + filename);
      return vector<char>(0);
    }
  } else {
    debug::Log("NitroFS Open FAILED for " + filename);
    return vector<char>(0);
  }
}

// Note: while this performs sanity checks on the file reading)
void LoadEntireFileIntoMem(string filename, char* destination_buffer, int max_size) {
  auto file = fopen(filename.c_str(), "rb");
  if (file) {
    fseek(file, 0, SEEK_END);
    auto const file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size > max_size) {
      debug::Log("Load into Mem failed for " + filename);
      debug::Log("Attempted to read " + std::to_string(file_size) + "bytes");
      debug::Log("Buffer can only hold " + std::to_string(max_size) + "bytes");
      return;
    }

    if (fread(destination_buffer, 1, file_size, file)) {
      // Done!      
    } else {
      debug::Log("NitroFS Read FAILED for " + filename);
    }
  } else {
    debug::Log("NitroFS Open FAILED for " + filename);    
  }
}