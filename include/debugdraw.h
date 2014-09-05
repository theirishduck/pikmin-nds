#ifndef DEBUG_DRAW_H
#define DEBUG_DRAW_H

#include "vector.h"

namespace debug{

void drawCrosshair(Vec3 p, rgb color = RGB5(31,31,31));
void drawGroundPlane(int width, int segments, rgb color = RGB5(0,0,0));

}
#endif