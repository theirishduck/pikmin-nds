#include <nds.h>
#include <stdio.h>

#include <array>
#include <functional>

#include "MultipassEngine.h"
#include "RedPikmin.h"
#include "YellowPikmin.h"
#include "Captain.h"

//debug texture loading stuff
#include "piki_eyes_img_bin.h"

#include "BasicMechanics.h"

volatile int frame = 0;

#define TEST_PIKMIN 33

MultipassEngine engine;
RedPikmin red_pikmin[TEST_PIKMIN];
RedPikmin red_pikmin2[TEST_PIKMIN];
RedPikmin red_pikmin3[TEST_PIKMIN];
YellowPikmin yellow_pikmin[TEST_PIKMIN];
Captain captain[TEST_PIKMIN];
Captain captain2[TEST_PIKMIN];
Captain captain3[TEST_PIKMIN];

using namespace std;

void init() {
    //Do this manually, so we can use BANK_H instead of BANK_C
    vramSetBankH(VRAM_H_SUB_BG);
    videoSetModeSub(MODE_0_2D);
    consoleInit(NULL,
        0,
        BgType_Text4bpp,
        BgSize_T_256x256,
        15,
        0,
        false,
        true);

    printf("Multipass Engine Demo\n");
    
    videoSetMode(MODE_0_3D);
    glInit();
    glEnable(GL_TEXTURE_2D);
    
    // setup the rear plane
    glClearColor(4,4,4,31);
    glClearDepth(0x7FFF); //TODO: Play with this maybe? This might be why we were getting clipping at the back plane before.
    
    glViewport(0,0,255,191);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    gluPerspective(70, 256.0 / 192.0, 0.1, 4096);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    gluLookAt(0.0, 0.0, 10.0,  //camera position 
              0.0, 0.0, 0.0,   //look at
              0.0, 1.0, 0.0);  //up
    
    //Setup default lights; these may be overridden later
    glLight(0, RGB15(31,31,31) , floattov10(-0.40), floattov10(0.32), floattov10(0.27));
    glLight(1, RGB15(31,31,31) , floattov10(0.32), floattov10(0.32), floattov10(0.32));

    //setup demo pikmin
    for (int i = 0; i < TEST_PIKMIN; i++) {
        red_pikmin[i].setPosition({-5,0,-2 + i * -2});
        engine.addEntity(&red_pikmin[i]);

        red_pikmin2[i].setPosition({0,0,-2 + i * -2});
        engine.addEntity(&red_pikmin2[i]);
        if (i == 0) {
            //engine.targetEntity(&red_pikmin2[i]);
        }

        red_pikmin3[i].setPosition({5,0,-2 + i * -2});
        engine.addEntity(&red_pikmin3[i]);
    }
    
    captain[0].setPosition({0,1,0});
    captain[0].setAnimation("Armature|Idle1");
    engine.addEntity(&captain[0]);
    engine.targetEntity(&captain[0]);
    
    //copy the pikmin eye texture into VRAM, at the beginning of bank C
    //first, map that bank as CPU-accessible
    vramSetBankC(VRAM_C_LCD);
    //second, DMA the texture memory into place
    dmaCopy(piki_eyes_img_bin, VRAM_C, piki_eyes_img_bin_size);
    //finally, re-map bank C as texture memory
    vramSetBankC(VRAM_C_TEXTURE);

    glPushMatrix();
}

void gameloop() {
    frame++;
    
    touchPosition touchXY;
    touchRead(&touchXY);
    
    basicMechanicsUpdate();

    engine.update();
    engine.draw();
}

int main(void) {
  init();
  
  while(1) {
    gameloop();
  }

  return 0;
}
