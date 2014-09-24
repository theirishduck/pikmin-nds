#include "debugdraw.h"
#include <nds.h>

void debug::drawCrosshair(Vec3 p, rgb color) {
    glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
    glBegin(GL_TRIANGLE);
    glColor(color);
    glPushMatrix();
    glTranslatef32(p.x.data, p.y.data, p.z.data);
    glVertex3v16(0, -1 << 10, 0);
    glVertex3v16(0, 1 << 10, 0);
    glVertex3v16(0, 1 << 10, 0);

    glVertex3v16(1 << 10, 0, 0);
    glVertex3v16(-1 << 10, 0, 0);
    glVertex3v16(-1 << 10, 0, 0);

    glPopMatrix(1);
    glEnd();
}

void debug::drawGroundPlane(int width, int segments, rgb color) {
    rgb dark_color = (color & 0x7BDE) >> 1;
    glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE | (1<<12));
    glBegin(GL_TRIANGLE);
    glPushMatrix();
    glScalef(width / 2, 0, width / 2);
    for (int z = 0; z < segments; z++) {
        for (int x = 0; x < segments; x++) {
            if ((z + x) % 2 == 0) {
                glColor(color);
            } else {
                //glColor(color & 0xBDEF); //halve the color value
                //glColor(color & 0xDEF7); //approximately quarter the color value
                glColor(dark_color);
            }
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