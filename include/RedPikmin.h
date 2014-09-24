#ifndef REDPIKMIN_H
#define REDPIKMIN_H

#include "MultipassEngine.h"

class RedPikmin : public DrawableEntity {
    public:
        void update(MultipassEngine* engine);
        RedPikmin();
        ~RedPikmin();
        
    private:
        s16 rotation = 0;
        bool running = false;
        int nextTarget = 0;
        Vec3 target;
        Vec3 direction;
};

#endif