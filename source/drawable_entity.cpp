#include "drawable_entity.h"
#include "multipass_engine.h"
#include <stdio.h>
#include <nds/arm9/postest.h>

Vec3 DrawableEntity::position() {
    return current_.position;
}

void DrawableEntity::setPosition(Vec3 pos) {
    current_.position = pos;
}

Rotation DrawableEntity::rotation() {
    return current_.rotation;
}

void DrawableEntity::setRotation(int x, int y, int z) {
    current_.rotation.x = x;
    current_.rotation.y = y;
    current_.rotation.z = z;
}

DrawState DrawableEntity::getCachedState() {
    return cached_;
}

void DrawableEntity::setCache() {
    cached_ = current_;
}

void DrawableEntity::setActor(Dsgx* actor) {
    current_.actor = actor;
}

Dsgx* DrawableEntity::getActor() {
    return current_.actor;
}

void DrawableEntity::applyTransformation() {
    glTranslatef32(cached_.position.x.data_, cached_.position.y.data_, cached_.position.z.data_);
    
    //If the rotation value is zero, we skip the gl call; this doesn't affect
    //the end result, but DOES skip an expensive matrix transformation when possible.
    //This is very effective, since most of our rotations will only be about the Y axis;
    //Initial testing shows this reducing applyTransformation() CPU load by ~1/2 for typical scenes.
    if (cached_.rotation.y)
        glRotateYi(cached_.rotation.y);
    if (cached_.rotation.x)
        glRotateXi(cached_.rotation.x);
    if (cached_.rotation.z)
        glRotateZi(cached_.rotation.z);
}

void DrawableEntity::draw(MultipassEngine* engine) {
    //apply transformation
    //BG_PALETTE_SUB[0] = RGB5(31,31,0);
    applyTransformation();

    //if necessary, apply animation!
    if (cached_.animation) {
        //make sure the GFX engine is done drawing the previous object
        //while (GFX_STATUS & BIT(14)) {}
        //while (GFX_STATUS & BIT(27)) {}
        //BG_PALETTE_SUB[0] = RGB5(0,31,31);
        cached_.actor->applyAnimation(cached_.animation, cached_.animation_frame);
        //BG_PALETTE_SUB[0] = RGB5(0,0,31);
    }
    
    //draw the object!
    //BG_PALETTE_SUB[0] = RGB5(31,0,31);
    glCallList(cached_.actor->drawList());
    //BG_PALETTE_SUB[0] = RGB5(0,0,31);
}

void DrawableEntity::update(MultipassEngine* engine) {
    //if necessary, update animations
    if (current_.animation) {
        current_.animation_frame++;
        if (current_.animation_frame >= current_.animation->length) {
            current_.animation_frame = 0; //wrap around!
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
    PosTest(current_.actor->center().x.data_, current_.actor->center().y.data_, current_.actor->center().z.data_);
    //return THAT instead of the nonsense below
    Vec3 result;
    result.x.data_ = PosTestXresult();
    result.y.data_ = PosTestYresult();
    result.z.data_ = PosTestZresult();
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
        (current_.actor->center().x.data_ >> 6) * (clip[2] >> 6) + 
        (current_.actor->center().z.data_ >> 6) * (clip[6] >> 6) + //why is this Z used twice? Should this be Y here?
        (current_.actor->center().z.data_ >> 6) * (clip[10] >> 6) + 
        (floattov16(1.0) >> 6)     * (clip[14] >> 6);
        
    //printf("%f\n", ((float)cz) / (0x1 << 12));
    
    glPopMatrix(1);

    gx::Fixed<s32,12> thing;
    thing.data_ = (cz >> 1);
    BG_PALETTE_SUB[0] = RGB5(0,15,0);BG_PALETTE_SUB[0] = RGB5(0,15,0);
    return thing;
    */
}

void DrawableEntity::setAnimation(std::string name) {
    current_.animation = current_.actor->getAnimation(name);
    current_.animation_frame = 0;
}