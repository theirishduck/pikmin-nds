#include "DrawableEntity.h"
#include "MultipassEngine.h"
#include <stdio.h>

Vec3 DrawableEntity::position() {
    return current.position;
}

void DrawableEntity::setPosition(Vec3 pos) {
    current.position = pos;
}

Vec3 DrawableEntity::rotation() {
    return current.position;
}

void DrawableEntity::setRotation(Vec3 rot) {
    current.rotation = rot;
}

DrawState DrawableEntity::getCachedState() {
    return cached;
}

void DrawableEntity::setCache() {
    cached = current;
}

void DrawableEntity::setActor(DSGX* actor) {
    current.actor = actor;
}

void DrawableEntity::applyTransformation() {
    glTranslatef32(cached.position.x.data, cached.position.y.data, cached.position.z.data);
    
    glRotateY((float)cached.rotation.y);
    glRotateX((float)cached.rotation.x);
    glRotateZ((float)cached.rotation.z);
}

void DrawableEntity::draw(MultipassEngine* engine) {
    //apply transformation
    applyTransformation();
    
    //draw the object!
    glCallList(cached.actor->drawList());
}

gx::Fixed<s32,12> DrawableEntity::getRealModelCenter() {
    //avoid clobbering the render state for this poll
    glPushMatrix();
    applyTransformation();
    
    //wait for the matrix status to clear, and the geometry engine
    //to not be busy drawing (according to GBATEK, maybe not needed?)
    while (GFX_STATUS & BIT(14)) {}
    while (GFX_STATUS & BIT(27)) {}
    
    //read the clip coordinate matrix
    s32 clip[16];
    for (int i = 0; i < 16; i++)
        clip[i] = MATRIX_READ_CLIP[i];
    
    //multiply our current point by the clip matrix (warning: fixed point math
    //being done by hand here)
    s32 cz = 
        (current.actor->center().x.data >> 6) * (clip[2] >> 6) + 
        (current.actor->center().z.data >> 6) * (clip[6] >> 6) + //why is this Z used twice? Should this be Y here?
        (current.actor->center().z.data >> 6) * (clip[10] >> 6) + 
        (floattov16(1.0) >> 6)     * (clip[14] >> 6);
        
    //printf("%f\n", ((float)cz) / (0x1 << 12));
    
    glPopMatrix(1);

    gx::Fixed<s32,12> thing;
    thing.data = cz;
    return thing;
}