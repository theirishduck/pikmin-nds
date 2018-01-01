#include "level_loader.h"

#include <stdio.h>

#include "debug/messages.h"
#include "file_utils.h"
#include "numeric_types.h"
#include "pikmin_game.h"

using namespace std;

using numeric_types::literals::operator"" _f;
using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;
using numeric_types::fixed;

namespace level_loader {

const int kHeightmapBufferSize = 1024 * 512;
u8 heightmap_buffer[kHeightmapBufferSize];

void LoadLevel(PikminGame& game, std::string filename) {
	FILE* file = fopen(filename.c_str(), "r");
	char command_buffer[256];
	char arg_buffer[256];
	Handle last_handle;
	PikminGameState* last_object{nullptr};
	while (fscanf(file, "%s", command_buffer) > 0) {
		if (strcmp(command_buffer, "spawn") == 0) {
			fscanf(file, "%s", arg_buffer);
			debug::Log("Spawning: " + std::string(arg_buffer));
			last_handle = game.Spawn(arg_buffer);
			last_object = game.Retrieve(last_handle);
		} else if (strcmp(command_buffer, "position") == 0) {
			float x, y, z;
			fscanf(file, "%f %f %f", &x, &y, &z);

			if (last_object) {
				auto position = Vec3{fixed::FromFloat(x), fixed::FromFloat(y), fixed::FromFloat(z)};
				if (last_object->body) {
					last_object->set_position(position);
				} else {
					last_object->entity->set_position(position);
				}
			} else {
				debug::Log("Tried to set position after invalid spawn, ignoring.");
			}
		} else if (strcmp(command_buffer, "actor") == 0) {
			fscanf(file, "%s", arg_buffer);
			last_object->entity->set_actor(game.ActorAllocator()->Retrieve(arg_buffer));
		} else if (strcmp(command_buffer, "mesh") == 0) {
			fscanf(file, "%s", arg_buffer);
			last_object->entity->set_mesh(arg_buffer);
		} else if (strcmp(command_buffer, "heightmap") == 0) {
			fscanf(file, "%s", arg_buffer);
			LoadEntireFileIntoMem("/heightmaps/" + std::string(arg_buffer) + ".height", (char*)heightmap_buffer, kHeightmapBufferSize);
			game.world().SetHeightmap(heightmap_buffer);
			debug::Log("Set heightmap: " + std::string(arg_buffer));
		} else {
			debug::Log("Unrecognized command: " + std::string(command_buffer));
		}
	} 
}

} // namespace level_loader