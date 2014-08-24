#ifndef REDPIKMIN_H
#define REDPIKMIN_H

#include "MultipassEngine.h"

class RedPikmin : public DrawableEntity {
    public:
        void update(MultipassEngine* engine);
        
    private:
        v16 rotation = 0;
};

#endif