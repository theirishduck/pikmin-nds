#ifndef REDPIKMIN_H
#define REDPIKMIN_H

#include "MultipassEngine.h"

class RedPikmin : public DrawableEntity {
	public:
		RedPikmin(MultipassEngine* engine);
		void update();
		
	private:
		v16 rotation = 0;
};

#endif