#ifndef DRAWABLEENTITY_H
#define DRAWABLEENTITY_H
#include <nds.h>
#include "vector.h"

//Root for anything that the various Graphics Engines may use;
//intended to be inherited from to create game objects with
//custom logic.
struct DrawState {
    Vec3 position;
    Vec3 rotation;

    //TODO: make this reference an animation state instead?
    u32* model_data;
    v16 radius;
    int cull_cost; //in polygons
    Vector3<v16,12> model_center;
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
        
        Vec3 rotation();
        void setRotation(Vec3);
        
        void setActor(u32* model_data, Vector3<v16,12> model_center, v16 radius, int cull_cost);
        
        virtual void draw(MultipassEngine* engine);
        virtual void update(MultipassEngine* engine) {};
        virtual void applyTransformation();
        
        s32 getRealModelCenter();
};

#endif