#ifndef YELLOWPIKMIN_H
#define YELLOWPIKMIN_H

#include "MultipassEngine.h"

class YellowPikmin : public DrawableEntity {
    public:
        YellowPikmin();
        void update(MultipassEngine* engine);
        
    private:
        v16 rotation = 0;
};

#endif