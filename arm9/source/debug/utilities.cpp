#include "debug/utilities.h"
#include <cstdio>
#include <nds.h>
#include <map>

#include "pikmin_game.h"
#include "ai/captain.h"

using numeric_types::fixed;
using numeric_types::literals::operator"" _f;

void debug::nocashNumber(int num) {
  char buffer[20];
  sprintf(buffer, "%i", num);
  nocashMessage(buffer);
}

void debug::DrawLine(Vec2 p1, Vec2 p2, rgb color) {
  DrawLine(Vec3{p1.x, 0_f, p1.y}, Vec3{p2.x, 0_f, p2.y}, color);
}

void debug::DrawLine(Vec3 p1, Vec3 p2, rgb color) {
  glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
  glTexParameter(0, 0); //disable textures
  glBegin(GL_TRIANGLE);
  glColor(color);
  glPushMatrix();
  glTranslatef32(p1.x.data_, p1.y.data_ + (1 << 11), p1.z.data_);
  glVertex3v16(0, 0, 0);
  glPopMatrix(1);
  glPushMatrix();
  glTranslatef32(p2.x.data_, p2.y.data_ + (1 << 11), p2.z.data_);
  glVertex3v16(0, 0, 0);
  glVertex3v16(0, 0, 0);
  glPopMatrix(1);
  glEnd();
}

void debug::DrawCrosshair(Vec3 p, rgb color) {
  glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
  glTexParameter(0, 0); //disable textures
  glBegin(GL_TRIANGLE);
  glColor(color);
  glPushMatrix();
  glTranslatef32(p.x.data_, p.y.data_, p.z.data_);
  glVertex3v16(0, -1 << 10, 0);
  glVertex3v16(0, 1 << 10, 0);
  glVertex3v16(0, 1 << 10, 0);

  glVertex3v16(1 << 10, 0, 0);
  glVertex3v16(-1 << 10, 0, 0);
  glVertex3v16(-1 << 10, 0, 0);

  glPopMatrix(1);
  glEnd();
}

void debug::DrawCircle(Vec3 p, fixed radius, rgb color, u32 segments) {
  float const radiansPerArc = 360.0 / segments;

  glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
  glTexParameter(0, 0); //disable textures
  glBegin(GL_TRIANGLE);
  glColor(color);
  glPushMatrix();
  // We add 0.5 here to avoid a collision with the ground plane.
  glTranslatef32(p.x.data_, p.y.data_ + (1 << 11), p.z.data_);
  glScalef32(radius.data_,radius.data_,radius.data_);
  //spin right round
  for (u32 i = 0; i < segments; ++i) {
    glPushMatrix();
    glRotateY(i * radiansPerArc);
    glVertex3v16(1 << 12, 0, 0);
    glRotateY(radiansPerArc);
    glVertex3v16(1 << 12, 0, 0);
    glVertex3v16(1 << 12, 0, 0);
    glPopMatrix(1);
  }

  glPopMatrix(1);
  glEnd();

}

void debug::DrawGroundPlane(int width, int segments, rgb color) {
  // Derive a dark color by dividing each channel by 2. This is accomplished
  // using a bitmask: 0 rrrr0 gggg0 bbbb0, which removes the bottom bit in each
  // color channel. Shifting the result of this mask to the right results in
  // 0 0rrrr 0gggg 0bbbb, which is the desired result.
  rgb dark_color = (color & 0x7BDE) >> 1;
  glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE | (1 << 12));
  glTexParameter(0, 0); //disable textures
  glBegin(GL_TRIANGLE);
  glPushMatrix();
  glScalef(width / 2, 0, width / 2);
  for (int z = 0; z < segments; z++) {
    for (int x = 0; x < segments; x++) {
      glColor((z + x) % 2 == 0 ? color : dark_color);
      glVertex3f(-1.0f + (2.0f / segments) *  x     , 0,  -1.0f + (2.0f / segments) *  z);
      glVertex3f(-1.0f + (2.0f / segments) * (x + 1), 0,  -1.0f + (2.0f / segments) *  z);
      glVertex3f(-1.0f + (2.0f / segments) * (x + 1), 0,  -1.0f + (2.0f / segments) * (z + 1));

      glVertex3f(-1.0f + (2.0f / segments) * (x + 1), 0,  -1.0f + (2.0f / segments) * (z + 1));
      glVertex3f(-1.0f + (2.0f / segments) *  x     , 0,  -1.0f + (2.0f / segments) *  z);
      glVertex3f(-1.0f + (2.0f / segments) *  x     , 0,  -1.0f + (2.0f / segments) * (z + 1));
    }
  }

  glPopMatrix(1);
  glEnd();
}
