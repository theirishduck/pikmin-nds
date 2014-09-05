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
        int nextAnim = 0;
};

#endif