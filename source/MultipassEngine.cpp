#include "MultipassEngine.h"

//Used to increase the overlap between multipass slices.
//Might be useful to reduce visible seams. Higher values make the 3DS
//engine draw more extra polygons at the rear of a pass, and will likely
//cause artifacts with alpha-blended polygons.
#ifndef CLIPPING_FUDGE_FACTOR
#define CLIPPING_FUDGE_FACTOR 0
#endif

void MultipassEngine::addEntity(DrawableEntity* entity) {
	entities.push_back(entity);
}

void MultipassEngine::update() {
	for (auto entity = entities.begin(); entity != entities.end(); entity++) {
		(*entity)->update();
	}
}

void clipFriendly_Perspective(s32 near, s32 far, float angle)
{
	int ang = degreesToAngle(angle);
	int sine = sinLerp(ang);
	int cosine = sinLerp(ang);

	MATRIX_MULT4x4 = divf32((3 * cosine) , (4 * sine));
	MATRIX_MULT4x4 = 0;
	MATRIX_MULT4x4 = 0;
	MATRIX_MULT4x4 = 0;

	MATRIX_MULT4x4 = 0;
	MATRIX_MULT4x4 = divf32(cosine, sine);
	MATRIX_MULT4x4 = 0;
	MATRIX_MULT4x4 = 0;

	MATRIX_MULT4x4 = 0;
	MATRIX_MULT4x4 = 0;
	MATRIX_MULT4x4 = -divf32(far + near, far - near);
	MATRIX_MULT4x4 = floattof32(-1.0);

	MATRIX_MULT4x4 = 0;
	MATRIX_MULT4x4 = 0;
	MATRIX_MULT4x4 = -divf32(2 * mulf32(far, near), far - near);
	MATRIX_MULT4x4 = 0;
}

void MultipassEngine::gatherDrawList() {
	//First up, set our projection matrix to something normal, so we can sort the list properly (without clip plane distortion)
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	clipFriendly_Perspective(floattof32(0.1), floattof32(20.0), 70.0);
	glMatrixMode(GL_MODELVIEW);
	
	//cheat at cameras (TODO: NOT THIS)
	glLoadIdentity();
	gluLookAt(	0.0, 5.0, 6.0,		//camera possition
				0.0, 0.0, 0.5,		//look at
				0.0, 1.0, 0.0);		//up
				
	for (auto entity = entities.begin(); entity != entities.end(); entity++) {
		//cache this object, in case we need to reuse it for multiple passes
		(*entity)->setCache();
		DrawState state = (*entity)->getCachedState();
		
		//Using the camera state, calculate the nearest and farthest points,
		//which we'll later use to decide where the clipping planes should go.
		EntityContainer container;
		container.entity = *entity;
		s32 object_center = (*entity)->getRealModelCenter();
		container.far_z  = object_center - state.radius;
		container.near_z = object_center + state.radius;
		
		drawList.push(container);
	}
}

//setup 

void MultipassEngine::draw() {
	
	if (drawList.empty()) {
		//This is the first (and maybe last) frame of this pass, so
		//cache the draw state and set up the queue
		gatherDrawList();
		
		//to be extra sure, clear the overlap list
		//(it *should* be empty already at this point.)
		overlapList.clear();
		current_pass = 0;
		
		
	} else {
		//Enable the rear plane, using the last frame's render result
		//TODO: THIS
	}
	
	//PROCESS LIST
	int polycount = 0;
	
	while (!drawList.empty() && polycount < 1800) {
		EntityContainer container = drawList.top();
		drawList.pop();
		
		glPushMatrix();
		container.entity->draw();
		glPopMatrix(1);
		
		polycount += container.entity->getCachedState().cull_cost;
	}
	
	
	//TODO: THIS
	
	if (drawList.empty()) {
		///Draw directly to the screen, set up LCD capture to bank C
		//TODO: THIS
	} else {
		//This is a multipass frame; capture to either bank A or B
		//TODO: THIS
	}
	
	//make sure our draw calls get processed
	GFX_FLUSH = 0;
	
	return; //skip all that V
	
	
	//DEBUG STUFF
	//setup projection for the draw
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	clipFriendly_Perspective(floattof32(0.1), floattof32(20.0), 70.0);
	glMatrixMode(GL_MODELVIEW);
	
	//cheat at cameras
	glLoadIdentity();
	gluLookAt(	0.0, 5.0, 6.0,		//camera possition
				0.0, 0.0, 0.5,		//look at
				0.0, 1.0, 0.0);		//up
	
	
	//for now? ignore all that. pikmin. Onscreen. Now.
	for (auto entity = entities.begin(); entity != entities.end(); entity++) {
		(*entity)->setCache();
		
		glPushMatrix(); //preserve state
		(*entity)->draw();
		glPopMatrix(1); //restore state
	}
	GFX_FLUSH = 0;
}