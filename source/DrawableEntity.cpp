#include "DrawableEntity.h"
#include "MultipassEngine.h"
#include <stdio.h>
#include <nds/arm9/postest.h>

Vec3 DrawableEntity::position() {
    return current.position;
}

void DrawableEntity::setPosition(Vec3 pos) {
    current.position = pos;
}

Vec3 DrawableEntity::rotation() {
    return current.position;
}

void DrawableEntity::setRotation(int x, int y, int z) {
    current.rotation.x = x;
    current.rotation.y = y;
    current.rotation.z = z;
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

DSGX* DrawableEntity::getActor() {
    return current.actor;
}

void DrawableEntity::applyTransformation() {
    glTranslatef32(cached.position.x.data, cached.position.y.data, cached.position.z.data);
    
    //If the rotation value is zero, we skip the gl call; this doesn't affect
    //the end result, but DOES skip an expensive matrix transformation when possible.
    //This is very effective, since most of our rotations will only be about the Y axis;
    //Initial testing shows this reducing applyTransformation() CPU load by ~1/2 for typical scenes.
    if (cached.rotation.y)
        glRotateYi(cached.rotation.y);
    if (cached.rotation.x)
        glRotateXi(cached.rotation.x);
    if (cached.rotation.z)
        glRotateZi(cached.rotation.z);
}

void DrawableEntity::draw(MultipassEngine* engine) {
    //apply transformation
    //BG_PALETTE_SUB[0] = RGB5(31,31,0);
    applyTransformation();

    //if necessary, apply animation!
    if (cached.animation) {
        //make sure the GFX engine is done drawing the previous object
        //while (GFX_STATUS & BIT(14)) {}
        //while (GFX_STATUS & BIT(27)) {}
        //BG_PALETTE_SUB[0] = RGB5(0,31,31);
        cached.actor->applyAnimation(cached.animation, cached.animation_frame);
        //BG_PALETTE_SUB[0] = RGB5(0,0,31);
    }
    
    //draw the object!
    //BG_PALETTE_SUB[0] = RGB5(31,0,31);
    glCallList(cached.actor->drawList());
    //BG_PALETTE_SUB[0] = RGB5(0,0,31);
}

void DrawableEntity::update(MultipassEngine* engine) {
    //if necessary, update animations
    if (current.animation) {
        current.animation_frame++;
        if (current.animation_frame >= current.animation->length) {
            current.animation_frame = 0; //wrap around!
        }
    }
}

Vec3 DrawableEntity::getRealModelCenter() {
    //how long do we take transforming?
    //BG_PALETTE_SUB[0] = RGB5(31,31,0);
    //avoid clobbering the render state for this poll
    glPushMatrix();
    applyTransformation();
    //BG_PALETTE_SUB[0] = RGB5(0,31,0);
    
    //wait for the matrix status to clear, and the geometry engine
    //to not be busy drawing (according to GBATEK, maybe not needed?)
    while (GFX_STATUS & BIT(14)) {}
    while (GFX_STATUS & BIT(27)) {}
    
    //Run a POS_TEST
    PosTest(current.actor->center().x.data, current.actor->center().y.data, current.actor->center().z.data);
    //return THAT instead of the nonsense below
    Vec3 result;
    result.x.data = PosTestXresult();
    result.y.data = PosTestYresult();
    result.z.data = PosTestZresult();
    glPopMatrix(1);
    return result;

    /*
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
    thing.data = (cz >> 1);
    BG_PALETTE_SUB[0] = RGB5(0,15,0);BG_PALETTE_SUB[0] = RGB5(0,15,0);
    return thing;
    */
}

void DrawableEntity::setAnimation(std::string name) {
    current.animation = current.actor->getAnimation(name);
    current.animation_frame = 0;
}