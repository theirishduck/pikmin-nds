#include "drawable_entity.h"
#include "multipass_engine.h"
#include <stdio.h>
#include <nds/arm9/postest.h>

Vec3 DrawableEntity::position() {
    return current_.position;
}

void DrawableEntity::set_position(Vec3 pos) {
    current_.position = pos;
}

Rotation DrawableEntity::rotation() {
    return current_.rotation;
}

void DrawableEntity::set_rotation(numeric_types::Brads x, numeric_types::Brads y, numeric_types::Brads z) {
    current_.rotation.x = x;
    current_.rotation.y = y;
    current_.rotation.z = z;
}

DrawState DrawableEntity::GetCachedState() {
    return cached_;
}

void DrawableEntity::SetCache() {
    cached_ = current_;
}

void DrawableEntity::set_actor(Dsgx* actor) {
    current_.actor = actor;
}

Dsgx* DrawableEntity::actor() {
    return current_.actor;
}

void DrawableEntity::ApplyTransformation() {
    glTranslatef32(cached_.position.x.data_, cached_.position.y.data_, cached_.position.z.data_);
    
    //If the rotation value is zero, we skip the gl call; this doesn't affect
    //the end result, but DOES skip an expensive matrix transformation when possible.
    //This is very effective, since most of our rotations will only be about the Y axis;
    //Initial testing shows this reducing applyTransformation() CPU load by ~1/2 for typical scenes.
    if (cached_.rotation.y.data_)
        glRotateYi(cached_.rotation.y.data_);
    if (cached_.rotation.x.data_)
        glRotateXi(cached_.rotation.x.data_);
    if (cached_.rotation.z.data_)
        glRotateZi(cached_.rotation.z.data_);
}

void DrawableEntity::Draw(MultipassEngine* engine) {
    //apply transformation
    //BG_PALETTE_SUB[0] = RGB5(31,31,0);
    ApplyTransformation();

    //if necessary, apply animation!
    if (cached_.animation) {
        //make sure the GFX engine is done drawing the previous object
        //while (GFX_STATUS & BIT(14)) {}
        //while (GFX_STATUS & BIT(27)) {}
        //BG_PALETTE_SUB[0] = RGB5(0,31,31);
        cached_.actor->ApplyAnimation(cached_.animation, cached_.animation_frame);
        //BG_PALETTE_SUB[0] = RGB5(0,0,31);
    }
    
    //draw the object!
    //BG_PALETTE_SUB[0] = RGB5(31,0,31);
    glCallList(cached_.actor->DrawList());
    //BG_PALETTE_SUB[0] = RGB5(0,0,31);
}

void DrawableEntity::Update(MultipassEngine* engine) {
    //if necessary, update animations
    if (current_.animation) {
        current_.animation_frame++;
        if (current_.animation_frame >= current_.animation->length) {
            current_.animation_frame = 0; //wrap around!
        }
    }
}

Vec3 DrawableEntity::GetRealModelCenter() {
    //how long do we take transforming?
    //BG_PALETTE_SUB[0] = RGB5(31,31,0);
    //avoid clobbering the render state for this poll
    glPushMatrix();
    ApplyTransformation();
    //BG_PALETTE_SUB[0] = RGB5(0,31,0);
    
    //wait for the matrix status to clear, and the geometry engine
    //to not be busy drawing (according to GBATEK, maybe not needed?)
    while (GFX_STATUS & BIT(14)) {}
    while (GFX_STATUS & BIT(27)) {}
    
    //Run a POS_TEST
    PosTest(current_.actor->Center().x.data_, current_.actor->Center().y.data_, current_.actor->Center().z.data_);
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

void DrawableEntity::SetAnimation(std::string name) {
    current_.animation = current_.actor->GetAnimation(name);
    current_.animation_frame = 0;
}