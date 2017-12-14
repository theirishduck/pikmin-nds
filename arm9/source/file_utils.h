#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string>
#include <vector>

std::vector<std::string> FilesInDirectory(std::string path);
std::vector<char> LoadEntireFile(std::string filename);

#endif