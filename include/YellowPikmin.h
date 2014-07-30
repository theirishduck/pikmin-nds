#ifndef YELLOWPIKMIN_H
#define YELLOWPIKMIN_H

#include "MultipassEngine.h"

class YellowPikmin : public DrawableEntity {
	public:
		YellowPikmin(MultipassEngine* engine);
		void update();
		
	private:
		v16 rotation = 0;
};

#endif