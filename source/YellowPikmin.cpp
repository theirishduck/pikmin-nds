#include "YellowPikmin.h"
#include "pikmin_yellow_dsgx.h"

YellowPikmin::YellowPikmin(MultipassEngine* engine):DrawableEntity(engine) {
	u32* data = (u32*)pikmin_yellow_dsgx;
	
	Vector3<v16,12> model_center;
	model_center.x = floattov16(((float*)data)[0] / 4.0);
	model_center.y = floattov16(((float*)data)[1] / 4.0);
	model_center.z = floattov16(((float*)data)[2] / 4.0);
	v16 radius = floattov16(((float*)data)[3] / 4.0);
	int cull_cost = (int)data[4];
	
	setActor(
		&data[5], //start of model data, including number of commands
		model_center,
		radius,
		cull_cost);
}

void YellowPikmin::update() {
	setRotation({0,rotation,0});
	rotation -= 1;
}