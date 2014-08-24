#include "ProjectSettings.h"
#include "MultipassEngine.h"
#include <vector>
#include <stdio.h>

using namespace std;

void MultipassEngine::addEntity(DrawableEntity* entity) {
    entities.push_back(entity);
}

void MultipassEngine::update() {
    for (auto entity : entities) {
        entity->update(this);
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
    clipFriendly_Perspective(floattof32(0.1), floattof32(256.0), 70.0); //256 will be our backplane, and it's a good largeish number for reducing rouding errors
    glMatrixMode(GL_MODELVIEW);
    
    //cheat at cameras (TODO: NOT THIS)
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
        gx::Fixed<s32,12> object_center = entity->getRealModelCenter();
        container.far_z  = object_center + state.actor->radius();
        container.near_z = object_center - state.actor->radius();
        
        drawList.push(container);
        
        //debug!
        //printf("far: %f\n", ((float)container.far_z) / (0x1 << 12));
        //printf("near: %f\n", ((float)container.near_z) / (0x1 << 12));
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
    gluLookAt(  0.0, 3.0, 4.0,      //camera possition
                0.0, 3.0, 0.5,      //look at
                0.0, 1.0, 0.0);     //up
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
        //This is the first (and maybe last) frame of this pass, so
        //cache the draw state and set up the queue
        gatherDrawList();
        
        //to be extra sure, clear the overlap list
        //(it *should* be empty already at this point.)
        overlap_list.clear();
        current_pass = 0;
        
        printf("\x1b[2J");
    }
    
    
    
    //PROCESS LIST
    int polycount = 0;
    unsigned int initial_length = drawList.size();
    
    //Come up with a pass_list; how many objects can we draw in a single frame?
    vector<EntityContainer> pass_list;
    
    //if there are any overlap objects, we need to start by re-drawing those
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
    gx::Fixed<s32,12> far_plane = pass_list.front().far_z;
    if (far_plane > 256.0f) {
        far_plane = 256.0f; //real back clip plane; actually cull objects here regardless of what the engine thinks
    }
    gx::Fixed<s32,12> near_plane = 0.1f;
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
    
    //set the new projection and camera matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    near_plane = 0.1f;
    far_plane = 256.0f;
    clipFriendly_Perspective(near_plane.data, far_plane.data, 70.0);
    //clipFriendly_Perspective(floattof32(0.1), floattof32(256.0), 70.0);
    printf("near: %f\n", (float)near_plane);
    printf("far: %f\n", (float)far_plane);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    applyCameraTransform();
    
    //actually draw the pass_list
    for (auto& container : pass_list) {
        glPushMatrix();
        container.entity->draw(this);
        glPopMatrix(1);
        
        //if this object is not fully drawn, add it to the overlap list for the next pass
        if (container.near_z < near_plane && near_plane > floattof32(0.1)) {
            overlap_list.push_back(container);
        }
    }
    
    //if necessary, draw the clear plane
    drawClearPlane();
    
    printf("%d: ", current_pass);
    printf("objs: %d, ", pass_list.size());
    printf("repeats: %d\n", overlap_list.size());
    
    
    //make sure our draw calls get processed
    GFX_FLUSH = 0;
    swiWaitForVBlank();
    
    setVRAMforPass(current_pass);
    current_pass++;
    
    return;
}