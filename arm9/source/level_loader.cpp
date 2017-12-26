#include "level_loader.h"

#include <stdio.h>

#include "debug/messages.h"
#include "pikmin_game.h"

using namespace std;

extern "C" void __sync_synchronize() {}

void LoadLevel(PikminGame& game, std::string filename) {
	FILE* file = fopen(filename.c_str(), "r");
	char input_buffer[256];
	while (fscanf(file, "%s", input_buffer) > 0) {
		if (strcmp(input_buffer, "spawn") == 0) {
			fscanf(file, "%s", input_buffer);
			debug::Log("Would Spawn: " + std::string(input_buffer));
		} else {
			debug::Log("Unrecognized command: " + std::string(input_buffer));
		}
	} 
}