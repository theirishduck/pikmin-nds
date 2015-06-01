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
        glPolyFmt(POLY_ALPHA(alpha) | POLY_CULL_BACK);
        /*glTexParameter(0,
          ((((u32)particle.texture.offset) / 8) & 0xFFFF) |
          (particle.texture.format_width << 20) | 
          (particle.texture.format_height << 23) | 
          (GL_RGBA << 26)
          );
          */
        *((u32*)0x40004A8) = 
          ((((u32)particle.texture.offset) / 8) & 0xFFFF) |
          (particle.texture.format_width << 20) | 
          (particle.texture.format_height << 23) | 
          (GL_RGBA << 26);

        glPushMatrix();
        glTranslatef32(particle.position.x.data_, particle.position.y.data_,
          particle.position.z.data_);
        glRotateYi(y_angle.data_);
        glRotateXi(x_angle.data_);

        glBegin(GL_QUAD);
        glColor(RGB15(31,31,31));
        glTexCoord2t16(0, 0);
        glVertex3v16(-1 << 12,  1 << 12, 0);
        glTexCoord2t16((8 << particle.texture.format_width) << 4,  0);
        glVertex3v16( 1 << 12,  1 << 12, 0);
        glTexCoord2t16((8 << particle.texture.format_width) << 4,  (8 << particle.texture.format_height) << 4);
        glVertex3v16( 1 << 12, -1 << 12, 0);
        glTexCoord2t16(0,  (8 << particle.texture.format_height) << 4);
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
