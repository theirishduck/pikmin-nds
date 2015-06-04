#include "particle.h"

#include <nds.h>
#include "numeric_types.h"

using numeric_types::literals::operator"" _f;
using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;

Particle g_particles[128];

void UpdateParticles() {
  for (int slot = 0; slot < 128; slot++) {
    Particle& particle = g_particles[slot];
    if (particle.active) {
      particle.age++;
      if (particle.age > particle.lifespan) {
        particle.active = false;
      } else {
        particle.position += particle.velocity;
        particle.velocity += particle.acceleration;
        particle.alpha = particle.alpha - particle.fade_rate;
      }
    }
  }
}

void DrawParticles(Vec3 camera_position, Vec3 target_position) {
  // figure out the angle toward the camera (From the target; this will
  // end up being shared among all particles)
  Brads x_angle;
  Brads y_angle;
  auto difference_xz = Vec2{camera_position.x, camera_position.z} - 
      Vec2{target_position.x, target_position.z};
  if (difference_xz.Length2() > 0_f) {
    // Rotate about the Y axis to face the camera
    auto difference = difference_xz.Normalize();
    if (difference.y <= 0_f) {
      y_angle = Brads::Raw(acosLerp(difference.x.data_)) + 270_brad;
    } else {
      y_angle = Brads::Raw(-acosLerp(difference.x.data_)) + 270_brad;
    }

    // Use the distance to figure out rotation about the X axis
    auto xz_length = difference_xz.Length();
    difference = Vec2{xz_length, -camera_position.y}.Normalize();
    if (difference.y <= 0_f) {
      x_angle = Brads::Raw(acosLerp(difference.x.data_));
    } else {
      x_angle = Brads::Raw(-acosLerp(difference.x.data_));
    }
  }

  for (int slot = 0; slot < 128; slot++) {
    Particle& particle = g_particles[slot];
    if (particle.active) {
      int alpha = (int)(particle.alpha * 31_f);
      if (alpha > 31) {
        alpha = 31;
      }
      if (alpha > 1) {
        //glPolyFmt(POLY_ALPHA(alpha) | POLY_CULL_BACK);
        glPolyFmt(POLY_ALPHA(alpha) | POLY_ID(slot) | POLY_CULL_BACK);
        // Note: The OpenGL functions depend on internal state, and using them
        // here would cause a lot of overhead, so we're writing to the
        // registers manually.
        glBegin(GL_QUAD);
        // TEXIMAGE_PARAM
        *((u32*)0x40004A8) = 
          ((((u32)particle.texture.offset) / 8) & 0xFFFF) |
          (TEXTURE_SIZE_32 << 20) | 
          (TEXTURE_SIZE_32 << 23) | 
          (particle.texture.format << 26) | 
          (particle.texture.transparency << 29);

        // PLTT_BASE
        if (particle.texture.format == GL_RGB4) {
          *((u32*)0x40004AC) = 
            ((u32)particle.palette.offset - (u32)VRAM_G) / 8;
        } else {
          *((u32*)0x40004AC) = 
            ((u32)particle.palette.offset - (u32)VRAM_G) / 16;
        }

        glPushMatrix();
        glTranslatef32(particle.position.x.data_, particle.position.y.data_,
          particle.position.z.data_);
        glRotateYi(y_angle.data_);
        glRotateXi(x_angle.data_);

        glColor(RGB15(31,31,31));
        glTexCoord2t16(0, 0);
        glVertex3v16(-1 << 12,  1 << 12, 0);
        glTexCoord2t16((32) << 4,  0);
        glVertex3v16( 1 << 12,  1 << 12, 0);
        glTexCoord2t16((32) << 4,  (32) << 4);
        glVertex3v16( 1 << 12, -1 << 12, 0);
        glTexCoord2t16(0,  (32) << 4);
        glVertex3v16(-1 << 12, -1 << 12, 0);
        glEnd();

        glPopMatrix(1);
      }
    }
  }
}

Particle* SpawnParticle(Particle& prototype) {
  //find the first unused slot and put the particle there
  for (int slot = 0; slot < 128; slot++) {
    if (!g_particles[slot].active) {
      g_particles[slot] = prototype;
      //initialize hidden / tracking parameters
      g_particles[slot].active = true;
      g_particles[slot].age = 0;

      return &g_particles[slot];
    }
  }
  return nullptr;
}

// Note: slow! for debugging only; don't rely on this for gameplay.
int ActiveParticles() {
  int active = 0;
  for (int slot = 0; slot < 128; slot++) {
    if (g_particles[slot].active) {
      active++;
    }
  }
  return active;
}