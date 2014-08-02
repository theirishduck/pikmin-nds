#include <nds.h>
#include <stdio.h>

#include "MultipassEngine.h"
#include "RedPikmin.h"
#include "YellowPikmin.h"

volatile int frame = 0;

#define TEST_PIKMIN 20

using namespace std;

void init() {
  consoleDemoInit();

  printf("Multipass Engine Demo\n");
  
  videoSetMode(MODE_0_3D);
  glInit();
  glEnable(GL_TEXTURE_2D);
  
  // setup the rear plane
  glClearColor(0,0,31,31);
  glClearDepth(0x7FFF);
  
  glViewport(0,0,255,191);
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  
  gluPerspective(70, 256.0 / 192.0, 0.1, 4096);
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  gluLookAt(0.0, 0.0, 10.0,  //camera possition 
            0.0, 0.0, 0.0,   //look at
            0.0, 1.0, 0.0);  //up
  
  //Setup default lights; these may be overridden later
  glLight(0, RGB15(31,31,31) , floattov10(-0.5), floattov10(0.5), 0);
  glLight(1, RGB15(6,6,6)    , 0               , floattov10(-1.0), 0);
  glLight(2, RGB15(0,31,0) ,   floattov10(-1.0), 0,          0);
  glLight(3, RGB15(0,0,31) ,   floattov10(1.0) - 1,  0,          0);
  
  glPushMatrix();
}

void drawTriangleEntity(u8 r1, u8 g1, u8 b1, u8 r2, u8 g2, u8 b2, u8 r3, u8 g3, u8 b3) {
  glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
  glBegin(GL_TRIANGLE);
  glColor3b(r1, g1, b1);
  glVertex3v16(inttov16(-1),inttov16(-1),0);
  glColor3b(r2, g2, b2);
  glVertex3v16(inttov16(1),inttov16(-1),0);
  glColor3b(r3, g3, b3);
  glVertex3v16(inttov16(0),inttov16(1),0);
  glEnd();
}

void gameloop() {
  //Example debug code; remove later?
  frame++;
  
  touchPosition touchXY;
  touchRead(&touchXY);

  // print at using ansi escape sequence \x1b[line;columnH 
  //printf("\x1b[10;0HFrame = %d",frame);
  //printf("\x1b[16;0HTouch x = %04X, %04X\n", touchXY.rawx, touchXY.px);
  //printf("Touch y = %04X, %04X\n", touchXY.rawy, touchXY.py);

  // glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
  // glBegin(GL_TRIANGLE);
  // glColor3b(255,0,0);
  // glVertex3v16(inttov16(-1),inttov16(-1),0);
  // glColor3b(0,255,0);
  // glVertex3v16(inttov16(1),inttov16(-1),0);
  // glColor3b(0,0,255);
  // glVertex3v16(inttov16(0),inttov16(1),0);
  // glEnd();

  drawTriangleEntity(255, 0, 0, 0, 255, 0, 0, 0, 255);
  glFlush(0);
  swiWaitForVBlank();
}
  
int main(void) {
  init();
  
  while(1) {
    gameloop();
  }

  return 0;
}
