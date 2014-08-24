#ifndef CAPTAIN_H
#define CAPTAIN_H

#include "MultipassEngine.h"

class Captain : public DrawableEntity {
    public:
        void update(MultipassEngine* engine);
        
    private:
        v16 rotation = 0;
};

#endif