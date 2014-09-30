#ifndef DRAWABLEENTITY_H
#define DRAWABLEENTITY_H
#include <nds.h>
#include <string>
#include "vector.h"

#include "dsgx.h"

//Root for anything that the various Graphics Engines may use;
//intended to be inherited from to create game objects with
//custom logic.
struct Rotation {
        int x;
        int y;
        int z;
};

struct DrawState {
    Vec3 position;
    Rotation rotation;
    

    //TODO: make this reference an animation state instead?
    DSGX* actor;
    Animation* animation = 0;
    u32 animation_frame = 0;
};

class MultipassEngine;

class DrawableEntity {
    private:
        DrawState current;
        DrawState cached;
    
    public:
        DrawState getCachedState();
        void setCache();
        
        Vec3 position();
        void setPosition(Vec3);
        
        Rotation rotation();
        void setRotation(int x, int y, int z);
        
        void setActor(DSGX* actor);
        DSGX* getActor();
        
        virtual void draw(MultipassEngine* engine);
        virtual void update(MultipassEngine* engine);
        virtual void applyTransformation();
        
        Vec3 getRealModelCenter();

        void setAnimation(std::string name);
};

#endif