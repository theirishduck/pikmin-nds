#include "debugdraw.h"
#include <nds.h>

void debug::drawCrosshair(Vec3 p) {
    glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
    glBegin(GL_TRIANGLE);
    glColor3b(255, 255, 255);
    glPushMatrix();
    glTranslatef32(p.x.data, p.y.data, p.z.data);
    glVertex3v16(0, -1 << 11, 0);
    glVertex3v16(0, 1 << 11, 0);
    glVertex3v16(0, 1 << 11, 0);

    glVertex3v16(1 << 11, 0, 0);
    glVertex3v16(-1 << 11, 0, 0);
    glVertex3v16(-1 << 11, 0, 0);

    glPopMatrix(1);
    glEnd();
}