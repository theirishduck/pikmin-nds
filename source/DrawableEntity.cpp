#include "DrawableEntity.h"
#include "MultipassEngine.h"
#include <stdio.h>

DrawableEntity::DrawableEntity(MultipassEngine* e) {
	this->engine = engine;
}

Vec3 DrawableEntity::position() {
	return current.position;
}

void DrawableEntity::setPosition(Vec3 pos) {
	current.position = pos;
}

Vec3 DrawableEntity::rotation() {
	return current.position;
}

void DrawableEntity::setRotation(Vec3 rot) {
	current.rotation = rot;
}

DrawState DrawableEntity::getCachedState() {
	return cached;
}

void DrawableEntity::setCache() {
	cached = current;
}

void DrawableEntity::setActor(u32* model_data, Vector3<v16,12> model_center, v16 radius, int cull_cost) {
	current.model_data = model_data;
	current.radius = radius;
	current.cull_cost = cull_cost;
	current.model_center = model_center;
}

void DrawableEntity::draw() {
	//apply transformation
	glTranslatef(cached.position.x, cached.position.y, cached.position.z);
	
	glRotateY(cached.rotation.y);
	glRotateX(cached.rotation.x);
	glRotateZ(cached.rotation.z);
	
	//draw the object!
	glCallList(cached.model_data);
}

s32 DrawableEntity::getRealModelCenter() {
	//wait for the matrix status to clear, and the geometry engine
	//to not be busy drawing (according to GBATEK, maybe not needed?)
	while (GFX_STATUS & BIT(14)) {}
	while (GFX_STATUS & BIT(27)) {}
	
	//read the clip coordinate matrix
	s32 clip[16];
	for (int i = 0; i < 16; i++)
		clip[i] = MATRIX_READ_CLIP[i];
	
	//multiply our current point by the clip matrix (warning: fixed point math
	//being done by hand here)
	s32 cz = 
		(current.model_center.x >> 6) * (clip[2] >> 6) + 
		(current.model_center.z >> 6) * (clip[6] >> 6) + //why is this Z used twice? Shouldn't this be Y here?
		(current.model_center.z >> 6) * (clip[10] >> 6) + 
		(floattov16(1.0) >> 6)     * (clip[14] >> 6) ;
		
	return cz;
}