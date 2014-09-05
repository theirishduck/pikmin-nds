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
    glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
    glBegin(GL_QUAD);
    glColor(color);
    glPushMatrix();
    glScalef(width / segments, 0, width / segments);
    for (int z = 0; z < segments; z++) {
        for (int x = 0; x < segments; x++) {
            //glColor(rand());
            glVertex3f(-1.0f + (2.0f / segments) *  x     , 0,  -1.0f + (2.0f / segments) *  z);
            glVertex3f(-1.0f + (2.0f / segments) * (x + 1), 0,  -1.0f + (2.0f / segments) *  z);
            glVertex3f(-1.0f + (2.0f / segments) * (x + 1), 0,  -1.0f + (2.0f / segments) * (z + 1));
            glVertex3f(-1.0f + (2.0f / segments) *  x     , 0,  -1.0f + (2.0f / segments) * (z + 1));
        }
    }
    
    glPopMatrix(1);
    glEnd();
}