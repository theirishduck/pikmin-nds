#include "project_settings.h"
#include "multipass_engine.h"
#include <vector>
#include <stdio.h>

#include "debug_draw.h"

using namespace std;

MultipassEngine::MultipassEngine() {
    camera_position_destination = Vec3{0.0f, 6.0f, 4.0f};
    camera_target_destination   = Vec3{0.0f, 3.0f, 0.5f};

    camera_position_current = camera_position_destination;
    camera_target_current = camera_target_destination;
}

void MultipassEngine::targetEntity(DrawableEntity* entity) {
    entity_to_follow = entity;
}

void MultipassEngine::updateCamera() {
    if (keysDown() & KEY_R) {
        if (keysHeld() & KEY_L) {
            cameraDistance += 1;
            if (cameraDistance > 3) {
                cameraDistance = 1;
            }
        } else {
            highCamera = !highCamera;
        }
    }

    if (entity_to_follow) {
        float height = 2.5f + 2.5f * cameraDistance;
        if (highCamera) {
            height = 7.5f + 7.5f * cameraDistance;
        }

        if (keysDown() & KEY_L) {
            //move the camera directly behind the target entity,
            //based on their current rotation
            camera_position_destination = entity_to_follow->position();
            camera_position_destination.x.data -= cosLerp(entity_to_follow->rotation().y - degreesToAngle(90));
            camera_position_destination.z.data -= -sinLerp(entity_to_follow->rotation().y - degreesToAngle(90));
        }
        
        float follow_distance = 4.0f + 6.0f * cameraDistance;

        camera_target_destination = entity_to_follow->position();
        Vec3 entity_to_camera = entity_to_follow->position() - camera_position_destination;
        entity_to_camera.y = 0; //clear out height, so we work on the XZ plane.
        entity_to_camera = entity_to_camera.normalize();
        entity_to_camera = entity_to_camera * follow_distance;
        camera_position_destination = entity_to_follow->position() - entity_to_camera;
        camera_position_destination.y = height;

        printf("\x1b[8;0HC. Position: %.1f, %.1f, %.1f\n", (float)camera_position_destination.x, (float)camera_position_destination.y, (float)camera_position_destination.z);
        printf(       "C. Target  : %.1f, %.1f, %.1f\n", (float)camera_target_destination.x, (float)camera_target_destination.y, (float)camera_target_destination.z);
    } else {
        printf("No entity?\n");
    }

    camera_position_current = camera_position_destination * 0.25f + camera_position_current * 0.75f;
    camera_target_current = camera_target_destination * 0.25f + camera_target_current * 0.75f;
}

void MultipassEngine::setCamera(Vec3 position, Vec3 target) {
    camera_position_destination = position;
    camera_target_destination = target;
}

void MultipassEngine::addEntity(DrawableEntity* entity) {
    entities.push_back(entity);
}

void MultipassEngine::update() {
    scanKeys();

    for (auto entity : entities) {
        entity->update(this);
    }

    //its a SEEECRET
    if (keysDown() & KEY_A) {
        targetEntity(entities[rand() % entities.size()]);
    }

    updateCamera();

    //handle debugging features
    //TODO: make this more touchscreen-y and less basic?
    if ((keysHeld() & KEY_SELECT) && (keysDown() & KEY_A)) {
        debug_first_pass = !debug_first_pass;
        if (debug_first_pass) {
            printf("[DEBUG] Rendering only first pass.\n");
        } else {
            printf("[DEBUG] Rendering every pass.\n");
        }
    }

    if ((keysHeld() & KEY_SELECT) && (keysDown() & KEY_B)) {
        debug_timings = !debug_timings;
        if (debug_timings) {
            printf("[DEBUG] Render starting at scanline 0. (skipping vblank period.)\n");
        } else {
            printf("[DEBUG] Rendering starts immediately.\n");
        }
    }

    if ((keysHeld() & KEY_SELECT) && (keysDown() & KEY_X)) {
        debug_colors = !debug_colors;
        if (debug_colors) {
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
            return last_angle = 45;
        }
        if (keysHeld() & KEY_DOWN) {
            return last_angle = 315;
        }
        return last_angle = 0;
    }

    if (keysHeld() & KEY_LEFT) {
        if (keysHeld() & KEY_UP) {
            return last_angle = 135;
        }
        if (keysHeld() & KEY_DOWN) {
            return last_angle = 225;
        }
        return last_angle = 180;
    }

    if (keysHeld() & KEY_UP) {
        return last_angle = 90;
    }

    if (keysHeld() & KEY_DOWN) {
        return last_angle = 270;
    }

    return last_angle;
}

int MultipassEngine::cameraAngle() {
    Vec3 facing;
    facing = entity_to_follow->position() - camera_position_current;
    facing.y = 0; //work on the XZ plane
    if (facing.length() <= 0) {
        return 0;
    }
    facing = facing.normalize();

    //return 0;
    if (facing.z <= 0) {
        return acosLerp(facing.x.data);
    } else {
        return -acosLerp(facing.x.data);
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
                
    for (auto entity : entities) {
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
        
        drawList.push(container);
    }
}

void MultipassEngine::setVRAMforPass(int pass) {
    //Setup VRAM for the rear-plane captures. Basically, we'll swap between banks A and B
    //based on the parity of the frame, so that each rear plane is the result of the previous
    //frame's capture. (The rear plane for frame 0 is simply not drawn.)
    if ((current_pass & 0x1) == 0) {
        vramSetBankA(VRAM_A_LCD);
        vramSetBankB(VRAM_B_TEXTURE_SLOT0);
        REG_DISPCAPCNT = DCAP_BANK(0) | DCAP_ENABLE | DCAP_SRC(1) | DCAP_SIZE(3);
    } else {
        vramSetBankA(VRAM_A_TEXTURE_SLOT0);
        vramSetBankB(VRAM_B_LCD);
        REG_DISPCAPCNT = DCAP_BANK(1) | DCAP_ENABLE | DCAP_SRC(1) | DCAP_SIZE(3);
    }

    //if the drawList is empty, we're rendering to the main screen,
    //and capturing the final render
    if (drawList.empty()) {
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
        (float)camera_position_cached.x, (float)camera_position_cached.y, (float)camera_position_cached.z, 
        (float)camera_target_cached.x,   (float)camera_target_cached.y,   (float)camera_target_cached.z,
        0.0f, 1.0f, 0.0f);

}

void MultipassEngine::drawClearPlane() {
    if (current_pass == 0)
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
    if (drawList.empty()) {
        if (debug_colors)
            BG_PALETTE_SUB[0] = RGB5(0,15,0);

        //This is the first (and maybe last) frame of this pass, so
        //cache the draw state and set up the queue
        camera_position_cached = camera_position_current;
        camera_target_cached = camera_target_current;
        gatherDrawList();
        
        //to be extra sure, clear the overlap list
        //(it *should* be empty already at this point.)
        overlap_list.clear();
        current_pass = 0;
        
        //printf("\x1b[2J");
        if (debug_colors)
            BG_PALETTE_SUB[0] = RGB5(0,0,0);
    }
    
    //PROCESS LIST
    int polycount = 0;
    unsigned int initial_length = drawList.size();
    
    //Come up with a pass_list; how many objects can we draw in a single frame?
    pass_list.clear();
    
    if (debug_colors)
        BG_PALETTE_SUB[0] = RGB5(31,31,0);

    //if there are any overlap objects, we need to start by re-drawing those
    //int overlaps_count = overlap_list.size();
    for (auto entity : overlap_list) {
        pass_list.push_back(entity);
        polycount += pass_list.back().entity->getCachedState().actor->drawCost();
    }
    overlap_list.clear();
    
    //now proceed to add objects from the remaining objects in the real draw list
    while (!drawList.empty() && polycount < MAX_POLYGONS_PER_PASS) {
        pass_list.push_back(drawList.top());
        polycount += pass_list.back().entity->getCachedState().actor->drawCost();
        drawList.pop();
    }

    if (debug_colors)
            BG_PALETTE_SUB[0] = RGB5(0,0,0);
    
    //if our drawlist made no progress, we either drew no objects, or managed to somehow make no
    //meaningful progress this frame; either way, we bail early. (In the latter case, this will
    //prevent the engine from hanging if there are too many objects in a row or something.)
    if (drawList.size() == initial_length) {
        if (!drawList.empty()) {
            printf("Impossible pass detected!\n");
            
            //forcibly empty the draw list
            while (!drawList.empty()) {
                drawList.pop();
            }
        }
    
        GFX_FLUSH = 0;
        swiWaitForVBlank();
        return;
    }

    //using the pass list, we can set our near/far clip planes
    if (current_pass == 0) {
        far_plane = 256.0f;
    } else {
        far_plane = near_plane; //from the last pass
    }
    
    near_plane = 0.1f;
    if (!drawList.empty()) {
        //set this pass's near plane *behind* the very next object in the list; this is where we
        //need to clip all of the objects we have just drawn.
        near_plane = drawList.top().far_z;
        //yet! if for some weird reason that would put our near clip plane in negative space, well...
        //let's not do that!
        if (near_plane < 0.1f) {
            near_plane = 0.1f;
        }
    }

    if (near_plane == far_plane) {
        //One of two things has happened. (1) Most likely, we've run into the front of the screen. (2) In cases where
        //many objects are very close, it's possible that the engine has "made progress" but not enough progress to
        //advance the near plane in front of the previous pass's far plane. In any case, trying to draw the scene in 
        //this situation will result in very, VERY broken depth values, and will cause extreme overlapping artifacts. 
        //This is to be avoided at all costs. (We drop a frame here.)
        if (far_plane == 0.1f) {
            printf("\x1b[10;0H Hit front of screen!\n");
            //front of the screen hit. Frame is safe to draw (contains sensible render) so do so now.
            //forcibly empty the draw list
            while (!drawList.empty()) {
                drawList.pop();
            }
            //if necessary, draw the clear plane
            drawClearPlane();

            GFX_FLUSH = 0;
            if (debug_colors)
                BG_PALETTE_SUB[0] = RGB5(6,6,6);
            swiWaitForVBlank();
            if (debug_colors)
                BG_PALETTE_SUB[0] = RGB5(0,0,0);

            setVRAMforPass(current_pass);
            current_pass++;
            return;
        } else {
            //This is a BAD case. Nothing to do here but bail            
            printf("\x1b[10;0H Near/Far plane equal! BAD!\n");
            
            //forcibly empty the draw list
            while (!drawList.empty()) {
                drawList.pop();
            }
        
            GFX_FLUSH = 0;
            swiWaitForVBlank();
            return;
        }
    }
    
    //set the new projection and camera matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //near_plane = 0.1f;
    //far_plane = 256.0f;
    clipFriendly_Perspective(near_plane.data, far_plane.data, FIELD_OF_VIEW);
    //clipFriendly_Perspective(floattof32(0.1), floattof32(256.0), 70.0);
    printf("\x1b[%d;0H(%d)n: %.3f f: %.3f\n", current_pass + 1, current_pass, (float)near_plane, (float)far_plane);
    //printf("near: %f\n", (float)near_plane);
    //printf("far: %f\n", (float)far_plane);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    applyCameraTransform();
    
    //actually draw the pass_list
    if (debug_colors)
            BG_PALETTE_SUB[0] = RGB5(0,0,31);
    //int o = 0;
    for (auto& container : pass_list) {
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
        if (container.near_z < near_plane/* && near_plane > floattof32(0.1)*/) {
            overlap_list.push_back(container);
        }
    }
    if (debug_colors)
            BG_PALETTE_SUB[0] = RGB5(0,0,0);
    
    //Draw the ground plane for debugging
    //debug::drawGroundPlane(64,10, RGB5(0, 24 - current_pass * 6, 0));
    debug::drawGroundPlane(64,10, RGB5(0, 24, 0));
    void basicMechanicsDraw();
    basicMechanicsDraw();

    //if necessary, draw the clear plane
    drawClearPlane();
    
    //printf("%d: ", current_pass);
    //printf("objs: %d, ", pass_list.size());
    //printf("repeats: %d\n", overlap_list.size());
    
    //make sure our draw calls get processed
    GFX_FLUSH = 0;
    if (debug_colors)
            BG_PALETTE_SUB[0] = RGB5(6,6,6);
    swiWaitForVBlank();
    if (debug_colors)
            BG_PALETTE_SUB[0] = RGB5(0,0,0);
    
    //DEBUG!!
    //Empty the draw list; this effectively limits us to one pass, and we drop all the rest
    if (debug_first_pass) {
        while (!(drawList.empty())) {
            drawList.pop();
        }
    }

    setVRAMforPass(current_pass);
    current_pass++;
    
    //DEBUG TIMINGS: spin until scanline 0
    if (debug_timings) {
        while (REG_VCOUNT != 0) {}
        irqEnable(IRQ_HBLANK);
        swiIntrWait(1,IRQ_HBLANK);
    }

    return;
}