#include <nds.h>
#include <stdio.h>

#include <array>
#include <functional>

#include "MultipassEngine.h"
#include "RedPikmin.h"
#include "YellowPikmin.h"

#include "BasicMechanics.h"

volatile int frame = 0;

#define TEST_PIKMIN 20

using namespace std;

void init() {
  consoleDemoInit();

  videoSetMode(MODE_0_3D);
  glInit();
  glEnable(GL_TEXTURE_2D);
  
  // setup the rear plane
  glClearColor(1,1,0,31);
  glClearDepth(0x7FFF);
  
  glViewport(0,0,255,191);
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  
  gluPerspective(70, 256.0 / 192.0, 0.1, 4096);
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  gluLookAt(0.0, 5.0, 10.0,  //camera possition 
            0.0, 0.0, 0.0,   //look at
            0.0, 1.0, 0.0);  //up
  
  //Setup default lights; these may be overridden later
  glLight(0, RGB15(31,31,31) , floattov10(-0.5), floattov10(0.5), 0);
  glLight(1, RGB15(6,6,6)    , 0               , floattov10(-1.0), 0);
  glLight(2, RGB15(0,31,0) ,   floattov10(-1.0), 0,          0);
  glLight(3, RGB15(0,0,31) ,   floattov10(1.0) - 1,  0,          0);
  
  glPushMatrix();
}

void gameloop() {
  //Example debug code; remove later?
  frame++;
  
  touchPosition touchXY;
  touchRead(&touchXY);
  scanKeys();

  basicMechanicsUpdate();
  basicMechanicsDraw();

  swiWaitForVBlank();
}

int main(void) {
  init();
  
  while(1) {
    gameloop();
  }

  return 0;
}
