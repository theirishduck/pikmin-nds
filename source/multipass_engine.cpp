#include "project_settings.h"
#include "multipass_engine.h"

#include <stdio.h>

#include <vector>

#include <nds/interrupts.h>
#include <nds/system.h>
#include <nds/arm9/background.h>
#include <nds/arm9/input.h>

#include "debug.h"

using namespace std;
using numeric_types::literals::operator"" _f;
using fixed = numeric_types::Fixed<s32,12>;

MultipassEngine::MultipassEngine() {
    camera_position_destination_ = Vec3{0_f, 6_f, 4_f};
    camera_target_destination_   = Vec3{0_f, 3_f, 0_f};

    camera_position_current_ = camera_position_destination_;
    camera_target_current_ = camera_target_destination_;
}

void MultipassEngine::targetEntity(DrawableEntity* entity) {
    entity_to_follow_ = entity;
}

void MultipassEngine::updateCamera() {
    if (keysDown() & KEY_R) {
        if (keysHeld() & KEY_L) {
            camera_distance_ += 1;
            if (camera_distance_ > 3) {
                camera_distance_ = 1;
            }
        } else {
            high_camera_ = !high_camera_;
        }
    }

    if (entity_to_follow_) {
        fixed height = 2.5_f + 2.5_f * fixed::FromInt(camera_distance_);
        if (high_camera_) {
            height = 7.5_f + 7.5_f * fixed::FromInt(camera_distance_);
        }

        if (keysDown() & KEY_L) {
            //move the camera directly behind the target entity,
            //based on their current rotation
            camera_position_destination_ = entity_to_follow_->position();
            camera_position_destination_.x.data_ -= cosLerp(entity_to_follow_->rotation().y - degreesToAngle(90));
            camera_position_destination_.z.data_ -= -sinLerp(entity_to_follow_->rotation().y - degreesToAngle(90));
        }
        
        fixed follow_distance = 4.0_f + 6.0_f * fixed::FromInt(camera_distance_);

        camera_target_destination_ = entity_to_follow_->position();
        Vec3 entity_to_camera = entity_to_follow_->position() - camera_position_destination_;
        entity_to_camera.y = 0_f; //clear out height, so we work on the XZ plane.
        entity_to_camera = entity_to_camera.normalize();
        entity_to_camera = entity_to_camera * follow_distance;
        camera_position_destination_ = entity_to_follow_->position() - entity_to_camera;
        camera_position_destination_.y = height;

        printf("\x1b[8;0HC. Position: %.1f, %.1f, %.1f\n", (float)camera_position_destination_.x, (float)camera_position_destination_.y, (float)camera_position_destination_.z);
        printf(       "C. Target  : %.1f, %.1f, %.1f\n", (float)camera_target_destination_.x, (float)camera_target_destination_.y, (float)camera_target_destination_.z);
    } else {
        printf("No entity?\n");
    }

    camera_position_current_ = camera_position_destination_ * 0.25_f + camera_position_current_ * 0.75_f;
    camera_target_current_ = camera_target_destination_ * 0.25_f + camera_target_current_ * 0.75_f;
}

void MultipassEngine::setCamera(Vec3 position, Vec3 target) {
    camera_position_destination_ = position;
    camera_target_destination_ = target;
}

void MultipassEngine::addEntity(DrawableEntity* entity) {
    entities_.push_back(entity);
}

void MultipassEngine::update() {
    scanKeys();

    for (auto entity : entities_) {
        entity->update(this);
    }

    //its a SEEECRET
    if (keysDown() & KEY_A) {
        targetEntity(entities_[rand() % entities_.size()]);
    }

    updateCamera();

    //handle debugging features
    //TODO: make this more touchscreen-y and less basic?
    if ((keysHeld() & KEY_SELECT) && (keysDown() & KEY_A)) {
        debug_first_pass_ = !debug_first_pass_;
        if (debug_first_pass_) {
            printf("[DEBUG] Rendering only first pass.\n");
        } else {
            printf("[DEBUG] Rendering every pass.\n");
        }
    }

    if ((keysHeld() & KEY_SELECT) && (keysDown() & KEY_B)) {
        debug_timings_ = !debug_timings_;
        if (debug_timings_) {
            printf("[DEBUG] Render starting at scanline 0. (skipping vblank period.)\n");
        } else {
            printf("[DEBUG] Rendering starts immediately.\n");
        }
    }

    if ((keysHeld() & KEY_SELECT) && (keysDown() & KEY_X)) {
        debug_colors_ = !debug_colors_;
        if (debug_colors_) {
            printf("[DEBUG] Rendering Colors\n");
        } else {
            printf("[DEBUG] No more seizures!\n");
        }
    }
}

int MultipassEngine::dPadDirection()  {
    //todo: make this not suck?

    if (keysHeld() & KEY_RIGHT) {
        if (keysHeld() & KEY_UP) {
            return last_angle_ = 45;
        }
        if (keysHeld() & KEY_DOWN) {
            return last_angle_ = 315;
        }
        return last_angle_ = 0;
    }

    if (keysHeld() & KEY_LEFT) {
        if (keysHeld() & KEY_UP) {
            return last_angle_ = 135;
        }
        if (keysHeld() & KEY_DOWN) {
            return last_angle_ = 225;
        }
        return last_angle_ = 180;
    }

    if (keysHeld() & KEY_UP) {
        return last_angle_ = 90;
    }

    if (keysHeld() & KEY_DOWN) {
        return last_angle_ = 270;
    }

    return last_angle_;
}

int MultipassEngine::cameraAngle() {
    Vec3 facing;
    facing = entity_to_follow_->position() - camera_position_current_;
    facing.y = 0_f; //work on the XZ plane
    if (facing.length() <= 0_f) {
        return 0;
    }
    facing = facing.normalize();

    //return 0;
    if (facing.z <= 0_f) {
        return acosLerp(facing.x.data_);
    } else {
        return -acosLerp(facing.x.data_);
    }
}



void clipFriendly_Perspective(s32 near, s32 far, float angle)
{
    int ang = degreesToAngle(angle);
    int sine = sinLerp(ang);
    int cosine = sinLerp(ang);

    MATRIX_MULT4x4 = divf32((3 * cosine) , (4 * sine));
    MATRIX_MULT4x4 = 0;
    MATRIX_MULT4x4 = 0;
    MATRIX_MULT4x4 = 0;

    MATRIX_MULT4x4 = 0;
    MATRIX_MULT4x4 = divf32(cosine, sine);
    MATRIX_MULT4x4 = 0;
    MATRIX_MULT4x4 = 0;

    MATRIX_MULT4x4 = 0;
    MATRIX_MULT4x4 = 0;
    MATRIX_MULT4x4 = -divf32(far + near, far - near);
    MATRIX_MULT4x4 = floattof32(-1.0);

    MATRIX_MULT4x4 = 0;
    MATRIX_MULT4x4 = 0;
    MATRIX_MULT4x4 = -divf32(2 * mulf32(far, near), far - near);
    MATRIX_MULT4x4 = 0;
}

void MultipassEngine::gatherDrawList() {
    //First up, set our projection matrix to something normal, so we can sort the list properly (without clip plane distortion)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    clipFriendly_Perspective(floattof32(0.1), floattof32(256.0), FIELD_OF_VIEW); //256 will be our backplane, and it's a good largeish number for reducing rouding errors
    glMatrixMode(GL_MODELVIEW);
    
    //reset to a normal matrix, in prep for calculations
    glLoadIdentity();
    applyCameraTransform();
                
    for (auto entity : entities_) {
        //cache this object, in case we need to reuse it for multiple passes
        entity->setCache();
        DrawState state = entity->getCachedState();
        
        //Using the camera state, calculate the nearest and farthest points,
        //which we'll later use to decide where the clipping planes should go.
        EntityContainer container;
        container.entity = entity;

        Vec3 object_center = entity->getRealModelCenter();
        container.far_z  = object_center.z + state.actor->radius();
        container.near_z = object_center.z - state.actor->radius();

        //debug: draw object centers (roughly)
        /*
        object_center.z *= -1;
        glPushMatrix();
        glLoadIdentity();
        debug::drawCrosshair(object_center, RGB5(0,0,0));
        glPopMatrix(1);
        */
        
        draw_list_.push(container);
    }
}

void MultipassEngine::setVRAMforPass(int pass) {
    //Setup VRAM for the rear-plane captures. Basically, we'll swap between banks A and B
    //based on the parity of the frame, so that each rear plane is the result of the previous
    //frame's capture. (The rear plane for frame 0 is simply not drawn.)
    if ((current_pass_ & 0x1) == 0) {
        vramSetBankA(VRAM_A_LCD);
        vramSetBankB(VRAM_B_TEXTURE_SLOT0);
        REG_DISPCAPCNT = DCAP_BANK(0) | DCAP_ENABLE | DCAP_SRC(1) | DCAP_SIZE(3);
    } else {
        vramSetBankA(VRAM_A_TEXTURE_SLOT0);
        vramSetBankB(VRAM_B_LCD);
        REG_DISPCAPCNT = DCAP_BANK(1) | DCAP_ENABLE | DCAP_SRC(1) | DCAP_SIZE(3);
    }

    //if the draw_list_ is empty, we're rendering to the main screen,
    //and capturing the final render
    if (draw_list_.empty()) {
        vramSetBankD(VRAM_D_LCD);
        videoSetMode(MODE_0_3D);
        REG_DISPCAPCNT = DCAP_BANK(3) | DCAP_ENABLE | DCAP_SRC(1) | DCAP_SIZE(3);
    } else {
        vramSetBankD(VRAM_D_MAIN_BG_0x06000000);
        videoSetMode(MODE_3_3D);
        bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
        bgSetPriority(3, 0);
        bgSetPriority(0, 3);
    }
}

void MultipassEngine::applyCameraTransform() {
    //TODO: Make this not static
    /*gluLookAt(  0.0, 6.0, 4.0,      //camera possition
                0.0, 3.0, 0.5,      //look at
                0.0, 1.0, 0.0);     //up
    */
    gluLookAt(
        (float)camera_position_cached_.x, (float)camera_position_cached_.y, (float)camera_position_cached_.z, 
        (float)camera_target_cached_.x,   (float)camera_target_cached_.y,   (float)camera_target_cached_.z,
        0.0f, 1.0f, 0.0f);

}

void MultipassEngine::drawClearPlane() {
    if (current_pass_ == 0)
    {
        //don't use the rear plane on the first pass (show clear color instead)
        return;
    }
    
    //set us up for orthagonal projection, no translation:
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    clipFriendly_Perspective(floattof32(0.1), floattof32(768.0), 1000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    //now, send in the 4 points for this quad
    GFX_BEGIN = 1;
    
    //first, setup the texture format:
    GFX_TEX_FORMAT = 
        (0) | //texture offset
        (5 << 20) | //texture width (256)
        (5 << 23) | //texture height (256)
        
        (1 << 16) | //repeat mode?
        (1 << 17) |
        
        (7 << 26) ; //texture format (Direct Texture)
        
    //set up the poly format, just draw it "white"
    
    
    //disable lighting?
    glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK);
    glColor3b(255, 255, 255);

    glTranslatef(0.0, 0.0, -768.0);
    glScalef(1024.0, 768.0, 1.0);
    GFX_TEX_COORD = (TEXTURE_PACK(inttot16(0), inttot16(0)));
    glVertex3v16(floattov16(-1.0),  floattov16(1.0), floattov16(0.0) );
    
    GFX_TEX_COORD = (TEXTURE_PACK(inttot16(0), inttot16(192)));
    glVertex3v16(floattov16(-1.0),  floattov16(-1.0), floattov16(0.0) );
    
    GFX_TEX_COORD = (TEXTURE_PACK(inttot16(256), inttot16(192)));
    glVertex3v16(floattov16(1.0),   floattov16(-1.0), floattov16(0.0) );
    
    GFX_TEX_COORD = (TEXTURE_PACK(inttot16(256), inttot16(0)));
    glVertex3v16(floattov16(1.0),   floattov16(1.0), floattov16(0.0) );
    
    GFX_TEX_FORMAT = 0; //no textures
        
}

void MultipassEngine::draw() {
    if (draw_list_.empty()) {
        if (debug_colors_)
            BG_PALETTE_SUB[0] = RGB5(0,15,0);

        //This is the first (and maybe last) frame of this pass, so
        //cache the draw state and set up the queue
        camera_position_cached_ = camera_position_current_;
        camera_target_cached_ = camera_target_current_;
        gatherDrawList();
        
        //to be extra sure, clear the overlap list
        //(it *should* be empty already at this point.)
        overlap_list_.clear();
        current_pass_ = 0;
        
        //printf("\x1b[2J");
        if (debug_colors_)
            BG_PALETTE_SUB[0] = RGB5(0,0,0);
    }
    
    //PROCESS LIST
    int polycount = 0;
    unsigned int initial_length = draw_list_.size();
    
    //Come up with a pass_list_; how many objects can we draw in a single frame?
    pass_list_.clear();
    
    if (debug_colors_)
        BG_PALETTE_SUB[0] = RGB5(31,31,0);

    //if there are any overlap objects, we need to start by re-drawing those
    //int overlaps_count = overlap_list_.size();
    for (auto entity : overlap_list_) {
        pass_list_.push_back(entity);
        polycount += pass_list_.back().entity->getCachedState().actor->drawCost();
    }
    overlap_list_.clear();
    
    //now proceed to add objects from the remaining objects in the real draw list
    while (!draw_list_.empty() && polycount < MAX_POLYGONS_PER_PASS) {
        pass_list_.push_back(draw_list_.top());
        polycount += pass_list_.back().entity->getCachedState().actor->drawCost();
        draw_list_.pop();
    }

    if (debug_colors_)
            BG_PALETTE_SUB[0] = RGB5(0,0,0);
    
    //if our draw_list_ made no progress, we either drew no objects, or managed to somehow make no
    //meaningful progress this frame; either way, we bail early. (In the latter case, this will
    //prevent the engine from hanging if there are too many objects in a row or something.)
    if (draw_list_.size() == initial_length) {
        if (!draw_list_.empty()) {
            printf("Impossible pass detected!\n");
            
            //forcibly empty the draw list
            while (!draw_list_.empty()) {
                draw_list_.pop();
            }
        }
    
        GFX_FLUSH = 0;
        swiWaitForVBlank();
        return;
    }

    //using the pass list, we can set our near/far clip planes
    if (current_pass_ == 0) {
        far_plane_ = 256_f;
    } else {
        far_plane_ = near_plane_; //from the last pass
    }
    
    near_plane_ = 0.1_f;
    if (!draw_list_.empty()) {
        //set this pass's near plane *behind* the very next object in the list; this is where we
        //need to clip all of the objects we have just drawn.
        near_plane_ = draw_list_.top().far_z;
        //yet! if for some weird reason that would put our near clip plane in negative space, well...
        //let's not do that!
        if (near_plane_ < 0.1_f) {
            near_plane_ = 0.1_f;
        }
    }

    if (near_plane_ == far_plane_) {
        //One of two things has happened. (1) Most likely, we've run into the front of the screen. (2) In cases where
        //many objects are very close, it's possible that the engine has "made progress" but not enough progress to
        //advance the near plane in front of the previous pass's far plane. In any case, trying to draw the scene in 
        //this situation will result in very, VERY broken depth values, and will cause extreme overlapping artifacts. 
        //This is to be avoided at all costs. (We drop a frame here.)
        if (far_plane_ == 0.1_f) {
            printf("\x1b[10;0H Hit front of screen!\n");
            //front of the screen hit. Frame is safe to draw (contains sensible render) so do so now.
            //forcibly empty the draw list
            while (!draw_list_.empty()) {
                draw_list_.pop();
            }
            //if necessary, draw the clear plane
            drawClearPlane();

            GFX_FLUSH = 0;
            if (debug_colors_)
                BG_PALETTE_SUB[0] = RGB5(6,6,6);
            swiWaitForVBlank();
            if (debug_colors_)
                BG_PALETTE_SUB[0] = RGB5(0,0,0);

            setVRAMforPass(current_pass_);
            current_pass_++;
            return;
        } else {
            //This is a BAD case. Nothing to do here but bail            
            printf("\x1b[10;0H Near/Far plane equal! BAD!\n");
            
            //forcibly empty the draw list
            while (!draw_list_.empty()) {
                draw_list_.pop();
            }
        
            GFX_FLUSH = 0;
            swiWaitForVBlank();
            return;
        }
    }
    
    //set the new projection and camera matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //near_plane_ = 0.1f;
    //far_plane_ = 256.0f;
    clipFriendly_Perspective(near_plane_.data_, far_plane_.data_, FIELD_OF_VIEW);
    //clipFriendly_Perspective(floattof32(0.1), floattof32(256.0), 70.0);
    printf("\x1b[%d;0H(%d)n: %.3f f: %.3f\n", current_pass_ + 1, current_pass_, (float)near_plane_, (float)far_plane_);
    //printf("near: %f\n", (float)near_plane_);
    //printf("far: %f\n", (float)far_plane_);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    applyCameraTransform();
    
    //actually draw the pass_list_
    if (debug_colors_)
            BG_PALETTE_SUB[0] = RGB5(0,0,31);
    //int o = 0;
    for (auto& container : pass_list_) {
        glLight(0, RGB15(31,31,31) , floattov10(-0.40), floattov10(0.32), floattov10(0.27));
        glLight(1, RGB15(31,31,31) , floattov10(0.32), floattov10(0.32), floattov10(0.32));
        /*
        if (o++ < overlaps_count) {
            //do something flashy!
            glLight(0, RGB15(15,15,15) , floattov10(-0.40), floattov10(0.32), floattov10(0.27));
            glLight(1, RGB15(15,15,15) , floattov10(0.32), floattov10(0.32), floattov10(0.32));
        }*/

        glPushMatrix();
        container.entity->draw(this);
        glPopMatrix(1);
        
        //if this object is not fully drawn, add it to the overlap list for the next pass
        if (container.near_z < near_plane_/* && near_plane_ > floattof32(0.1)*/) {
            overlap_list_.push_back(container);
        }
    }
    if (debug_colors_)
            BG_PALETTE_SUB[0] = RGB5(0,0,0);
    
    //Draw the ground plane for debugging
    //debug::drawGroundPlane(64,10, RGB5(0, 24 - current_pass_ * 6, 0));
    debug::drawGroundPlane(64,10, RGB5(0, 24, 0));
    void basicMechanicsDraw();
    basicMechanicsDraw();

    //if necessary, draw the clear plane
    drawClearPlane();
    
    //printf("%d: ", current_pass_);
    //printf("objs: %d, ", pass_list_.size());
    //printf("repeats: %d\n", overlap_list_.size());
    
    //make sure our draw calls get processed
    GFX_FLUSH = 0;
    if (debug_colors_)
            BG_PALETTE_SUB[0] = RGB5(6,6,6);
    swiWaitForVBlank();
    if (debug_colors_)
            BG_PALETTE_SUB[0] = RGB5(0,0,0);
    
    //DEBUG!!
    //Empty the draw list; this effectively limits us to one pass, and we drop all the rest
    if (debug_first_pass_) {
        while (!(draw_list_.empty())) {
            draw_list_.pop();
        }
    }

    setVRAMforPass(current_pass_);
    current_pass_++;
    
    //DEBUG TIMINGS: spin until scanline 0
    if (debug_timings_) {
        while (REG_VCOUNT != 0) {}
        irqEnable(IRQ_HBLANK);
        swiIntrWait(1,IRQ_HBLANK);
    }

    return;
}