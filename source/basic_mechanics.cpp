#include "basic_mechanics.h"

#include <functional>

#include <nds/arm9/input.h>

using std::function;

template<typename T>
struct Offset {
  T x, y, z;
};

struct Pikmin {
  enum class Type : u8 {
    Red,
    Yellow,
    Blue
  };
  enum class State : u8 {
    Idle,
    Active
  };

  Type type;
  State state;
  Offset<float> position;
};

struct Squad {
  u32 size = 100;
  float additionalOffset = 1;
  Offset<float> position;
};

struct Captain {
  Offset<float> cursor;
  float maxDistanceFromCursor = 4;
  float moveRate = 0.15f;
  Offset<float> position;
  s16 angle = 0;

  Squad squad;
};

Captain redCaptain;

void drawTriangleEntity(u8 r1, u8 g1, u8 b1, u8 r2, u8 g2, u8 b2, u8 r3, u8 g3, u8 b3) {
  glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
  glBegin(GL_TRIANGLE);
  glColor3b(r1, g1, b1);
  glVertex3v16(0, 1_v16, -0.25_v16);
  glColor3b(r2, g2, b2);
  glVertex3v16(0, 0.5_v16, 0.25_v16);
  glColor3b(r3, g3, b3);
  glVertex3v16(0, 0, -0.25_v16);
  glEnd();
}

void drawCaptain(u8 r, u8 g, u8 b) {
  drawTriangleEntity(r, g, b, 171, 109, 51, 245, 245, 175);
}

void drawRedCaptain() {
  drawCaptain(255, 0, 0);
}

void drawBlueCaptain() {
  drawCaptain(0, 0, 255);
}

void drawGridCell(u8 brightness) {
  glPolyFmt(POLY_ALPHA(0) | POLY_CULL_NONE);
  glBegin(GL_QUAD);
  glColor3b(brightness, brightness, brightness);
  glVertex3v16(0, 0, 0);
  glVertex3v16(0, 0, 1_v16);
  glVertex3v16(1_v16, 0, 1_v16);
  glVertex3v16(1_v16, 0, 0);
  glEnd();
}

void drawGrid() {
  glPushMatrix();

  s32 gridSize = 16;

  glTranslatef(-(gridSize >> 1), 0, -(gridSize >> 1));
  for (int x = 0; x < gridSize; ++x) {
    for (int z = 0; z < gridSize; ++z) {
      drawGridCell(32);
      glTranslatef(1, 0, 0);
    }
    glTranslatef(-gridSize, 0, 1);
  }

  glPopMatrix(1);
}

void drawCursor(u8 cursorR, u8 cursorG, u8 cursorB, u8 pointR, u8 pointG, u8 pointB) {
  glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
  glBegin(GL_QUAD);
  glColor3b(cursorR, cursorG, cursorB);
  glVertex3v16(-0.5_v16, 0, -0.5_v16);
  glVertex3v16(-0.5_v16, 0, 0.5_v16);
  glVertex3v16(0.5_v16, 0, 0.5_v16);
  glVertex3v16(0.5_v16, 0, -0.5_v16);
  glBegin(GL_TRIANGLE);
  glColor3b(pointR, pointG, pointB);
  glVertex3v16(-0.18_v16, 1.5_v16, 0);
  glVertex3v16(0.18_v16, 1.5_v16, 0);
  glVertex3v16(0, 1_v16, 0);
  glEnd();
}

void withTranslation(float x, float y, float z, function<void()> f) {
  glPushMatrix();
  glTranslatef(x, y, z);
  f();
  glPopMatrix(1);
}

void withScale(float x, float y, float z, function<void()> f) {
  glPushMatrix();
  glScalef(x, y, z);
  f();
  glPopMatrix(1);
}

void drawCircle(u32 segments, u8 r, u8 g, u8 b) {
  float const radiansPerArc = 360.0 / segments;

  glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
  glBegin(GL_TRIANGLE);
  glColor3b(r, g, b);
  for (u32 i = 0; i < segments; ++i) {
    glPushMatrix();
    glRotateY(i * radiansPerArc);
    glVertex3v16(1_v16, 0, 0);
    glVertex3v16(1_v16, 0, 0);
    glRotateY(radiansPerArc);
    glVertex3v16(1_v16, 0, 0);
    glPopMatrix(1);
  }
  glEnd();
}

void drawVector(v16 x, v16 y, v16 z) {
  glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
  glBegin(GL_TRIANGLE);
  glColor3b(0, 255, 0);
  glVertex3v16(0, 0, 0);
  glVertex3v16(0, 0, 0);
  glVertex3v16(x, y, z);
  glEnd();
}

Offset<float> delta(Offset<float> const& begin, Offset<float> const& end) {
  Offset<float> deltaVector;
  deltaVector.x = end.x - begin.x;
  deltaVector.y = end.y - begin.y;
  deltaVector.z = end.z - begin.z;
  return deltaVector;
}

int32 magnitude(Offset<float> const& vector) {
  Offset<int32> fixedPointVector{
    int32FromFloat(vector.x),
    int32FromFloat(vector.y),
    int32FromFloat(vector.z)
  };
  return sqrtf32(mulf32(fixedPointVector.x, fixedPointVector.x) +
      mulf32(fixedPointVector.y, fixedPointVector.y) +
      mulf32(fixedPointVector.z, fixedPointVector.z));
}

Offset<float> direction(Offset<float> const& begin, Offset<float> const& end) {
  Offset<float> deltaVector = delta(begin, end);
  int32 fixedPointDelta[] = {
    int32FromFloat(deltaVector.x),
    int32FromFloat(deltaVector.y),
    int32FromFloat(deltaVector.z)
  };
  normalizef32(fixedPointDelta);
  Offset<float> direction;
  direction.x = floatFromInt32(fixedPointDelta[0]);
  direction.y = floatFromInt32(fixedPointDelta[1]);
  direction.z = floatFromInt32(fixedPointDelta[2]);
  return direction;
}

bool diagonalKeysHeld() {
  return (keysCurrent() & (KEY_LEFT | KEY_RIGHT)) and
      (keysCurrent() & (KEY_UP | KEY_DOWN));
}

void updateCaptain(Captain& captain) {
  // Move the cursor.
  // Diagonal moves should still cover the same amount of ground as when a
  // single direction is held, but if the move rate is blindly applied to both
  // directions, then the cursor winds up moving 1.4 times faster when moving
  // diagonally. Offset that with a multiplier - sin(45 degrees) == sqrt(2) / 2,
  // which will cause the cursor to move at a constant rate diagonally and in
  // the cardinal directions.
  float const directionMultiplier = diagonalKeysHeld() ? 0.70710678118 : 1;
  if (keysCurrent() & KEY_LEFT) {
    captain.cursor.x -= captain.moveRate * directionMultiplier * 2;
    captain.position.x -= captain.moveRate * directionMultiplier;
  }
  if (keysCurrent() & KEY_RIGHT) {
    captain.cursor.x += captain.moveRate * directionMultiplier * 2;
    captain.position.x += captain.moveRate * directionMultiplier;
  }
  if (keysCurrent() & KEY_UP) {
    captain.cursor.z -= captain.moveRate * directionMultiplier * 2;
    captain.position.z -= captain.moveRate * directionMultiplier;
  }
  if (keysCurrent() & KEY_DOWN) {
    captain.cursor.z += captain.moveRate * directionMultiplier * 2;
    captain.position.z += captain.moveRate * directionMultiplier;
  }
  
  // Calculate the direction the captain should face to look at the cursor
  Offset<float> towardCursor = direction(captain.position, captain.cursor);

  // Snap the cursor back to the range of the captain. This prevents the cursor
  // from running away in the case the captain can not make progress towards the
  // cursor.
  int32 const newDistanceFromCursor =
      magnitude(delta(captain.position, captain.cursor));
  if (int32FromFloat(captain.maxDistanceFromCursor) < newDistanceFromCursor) {
    captain.cursor.x = captain.position.x + towardCursor.x * captain.maxDistanceFromCursor;
    captain.cursor.y = captain.position.y + towardCursor.y * captain.maxDistanceFromCursor;
    captain.cursor.z = captain.position.z + towardCursor.z * captain.maxDistanceFromCursor;
  }

  // Calculate the angle the captain should face - this should always be toward
  // the cursor.
  // Do a dot product with straight forward (the direction the character faces
  // by defualt) and the direction of the cursor. Because two of the components
  // (x and y) are zero in the first vector, the only component that needs to be
  // multiplied for the dot product is the z.
  // Though, thinking about it, since the along-z axis vector is of unit length,
  // all this math is equivalent to just the z component of the "toward" vector.
  captain.angle = acosLerp(v16FromFloat(towardCursor.z)) *
      (towardCursor.x < 0 ? -1 : 1);

  // Move the squad toward the captain if they are out of range of the captain.
  Offset<float> towardCaptain = direction(captain.squad.position, captain.position);
  int32 distanceFromSquad = magnitude(delta(captain.squad.position, captain.position));
  float const squadRadiusWithOffset = captain.squad.additionalOffset + floatFromInt32(sqrtf32(divf32(captain.squad.size << 12, 3.14159265358979_f32)));
  if (int32FromFloat(squadRadiusWithOffset) < distanceFromSquad) {
    captain.squad.position.x += captain.moveRate * towardCaptain.x;
    captain.squad.position.y += captain.moveRate * towardCaptain.y;
    captain.squad.position.z += captain.moveRate * towardCaptain.z;
  }
}

void basicMechanicsUpdate() {
  updateCaptain(redCaptain);
}

void basicMechanicsDraw() {
  float const squadRadius = floatFromInt32(sqrtf32(divf32(redCaptain.squad.size << 12, 3.14159265358979_f32)));
  float const squadOffset =
      redCaptain.squad.additionalOffset + squadRadius;

  withTranslation(0, -0.1, 0, []() {
      drawGrid();
  });
  withTranslation(redCaptain.position.x, redCaptain.position.y,
      redCaptain.position.z, [&squadOffset]() {
        glPushMatrix();
        glRotateYi(redCaptain.angle);
        drawRedCaptain();
        glPopMatrix(1);
        Offset<float> towardCursor = direction(redCaptain.position, redCaptain.cursor);
        drawVector(v16FromFloat(towardCursor.x), v16FromFloat(towardCursor.y), v16FromFloat(towardCursor.z));
        withScale(squadOffset, 1, squadOffset, [](){
              drawCircle(16, 255, 0, 0);
            });
      });
  withTranslation(redCaptain.cursor.x, redCaptain.cursor.y, redCaptain.cursor.z,
      []() {
        drawCursor(255, 128, 0, 192, 192, 192);
        withScale(4, 4, 4, []() {
            drawCircle(16, 255, 128, 0);
          });
      });
  withTranslation(redCaptain.squad.position.x, redCaptain.squad.position.y,
      redCaptain.squad.position.z, [&squadRadius]() {
      drawCursor(0, 255, 0, 192, 192, 192);
      withScale(squadRadius, 1, squadRadius, []() {
            drawCircle(16, 0, 255, 0);
          });
      });
  // glFlush(0);
}
