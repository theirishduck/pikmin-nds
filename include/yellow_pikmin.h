#ifndef YELLOWPIKMIN_H
#define YELLOWPIKMIN_H

#include "multipass_engine.h"

class YellowPikmin : public DrawableEntity {
    public:
        void update(MultipassEngine* engine);
        
    private:
        v16 rotation = 0;
};

#endif