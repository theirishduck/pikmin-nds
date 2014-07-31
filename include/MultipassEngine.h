#ifndef MULTIPASSENGINE_H
#define MULTIPASSENGINE_H

#include <queue>
#include "DrawableEntity.h"

struct EntityContainer {
	DrawableEntity* entity;
	s32 near_z;
	s32 far_z;
	bool operator< (const EntityContainer& other) const {return far_z <  other.far_z;}
		
};

class MultipassEngine {
	private:
		std::priority_queue<EntityContainer> drawList;
		std::vector<DrawableEntity*> entities;
		
		std::vector<EntityContainer> overlapList;
		
		int current_pass = 0;
		
		void gatherDrawList();
		
	public:
		void drawEntity(DrawableEntity entity);
		
		//called during gameloop to run the engine
		void update();
		void draw();
		
		void addEntity(DrawableEntity* entity);
};

#endif