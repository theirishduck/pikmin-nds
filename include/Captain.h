#ifndef CAPTAIN_H
#define CAPTAIN_H

#include "multipass_engine.h"

class Captain : public DrawableEntity {
    public:
        void update(MultipassEngine* engine);
        Captain();
        ~Captain();

        
    private:
        v16 rotation = 0;

        int running = true;
        int current_angle;
};

#endif