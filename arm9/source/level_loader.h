#ifndef LEVEL_LOADER_H
#define LEVEL_LOADER_H

#include <string>

class PikminGame;

namespace level_loader {

void LoadLevel(PikminGame& game, std::string filename);

} // namespace level_loader

#endif