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