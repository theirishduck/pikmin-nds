#include "particle.h"
#include "project_settings.h"

#include <nds.h>
#include "numeric_types.h"

using numeric_types::literals::operator"" _f;
using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;

Particle g_particles[MAX_PARTICLES];

u16 color_blend(u16 a, u16 b, u8 weight) {
  auto a_red =    a & 0x001F;
  auto a_green = (a & 0x03E0) >> 5;
  auto a_blue =  (a & 0x7C00) >> 10;

  auto b_red =    b & 0x001F;
  auto b_green = (b & 0x03E0) >> 5;
  auto b_blue =  (b & 0x7C00) >> 10;

  return
      (  a_red   * (weight + 1) + b_red   * (31 - weight)) / 32 +
      (((a_green * (weight + 1) + b_green * (31 - weight)) / 32) << 5) +
      (((a_blue  * (weight + 1) + b_blue  * (31 - weight)) / 32) << 10);
}

void UpdateParticles() {
  for (int slot = 0; slot < MAX_PARTICLES; slot++) {
    Particle& particle = g_particles[slot];
    if (particle.active) {
      particle.age++;
      if (particle.age > particle.lifespan) {
        particle.active = false;
      } else {
        particle.position += particle.velocity;
        particle.velocity += particle.acceleration;
        particle.alpha = particle.alpha - particle.fade_rate;
        particle.scale = particle.scale + particle.scale_rate;
        particle.rotation += particle.rotation_rate;
        if (particle.color_change_rate) {
          particle.color_weight += particle.color_change_rate;
          if (particle.color_weight > 31) {
            particle.color_weight = 31;
            particle.color_change_rate *= -1;
          }
          if (particle.color_weight < 0) {
            particle.color_weight = 0;
            particle.color_change_rate *= -1;
          }
          particle.color = color_blend(particle.color_a, particle.color_b, particle.color_weight);
        }
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

  for (int slot = 0; slot < MAX_PARTICLES; slot++) {
    Particle& particle = g_particles[slot];
    if (particle.active) {
      int alpha = (int)(particle.alpha * 31_f);
      if (alpha > 31) {
        alpha = 31;
      }
      if (alpha > 1) {
        glPolyFmt(POLY_ALPHA(alpha) | POLY_ID((slot & 0x1F) | 0x20) | POLY_CULL_BACK);
        // Note: The OpenGL functions depend on internal state, and using them
        // here would cause a lot of overhead, so we're writing to the
        // registers manually.
        glBegin(GL_QUAD);
        // TEXIMAGE_PARAM
        *((u32*)0x40004A8) =
          ((((u32)particle.texture.offset) / 8) & 0xFFFF) |
          (particle.texture.format_width << 20) |
          (particle.texture.format_height << 23) |
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
        if (particle.rotation != 0_brad) {
          glRotateZi(particle.rotation.data_);
        }
        glScalef32(particle.scale.data_, particle.scale.data_, particle.scale.data_);

        auto texture_width = (8 << particle.texture.format_width);
        auto texture_height = (8 << particle.texture.format_height);

        glColor(particle.color);
        glTexCoord2t16(0, 0);
        glVertex3v16(-1 << 12,  1 << 12, 0);
        glTexCoord2t16((texture_width) << 4,  0);
        glVertex3v16( 1 << 12,  1 << 12, 0);
        glTexCoord2t16((texture_width) << 4,  (texture_height) << 4);
        glVertex3v16( 1 << 12, -1 << 12, 0);
        glTexCoord2t16(0,  (texture_height) << 4);
        glVertex3v16(-1 << 12, -1 << 12, 0);
        glEnd();

        glPopMatrix(1);
      }
    }
  }
}

Particle* SpawnParticle(Particle& prototype) {
  //find the first unused slot and put the particle there
  for (int slot = 0; slot < MAX_PARTICLES; slot++) {
    if (!g_particles[slot].active) {
      g_particles[slot] = prototype;
      //initialize hidden / tracking parameters
      g_particles[slot].active = true;
      g_particles[slot].age = 0;
      if (prototype.color_change_rate) {
        g_particles[slot].color = prototype.color_a;
      }

      return &g_particles[slot];
    }
  }
  return nullptr;
}

// Note: slow! for debugging only; don't rely on this for gameplay.
int ActiveParticles() {
  int active = 0;
  for (int slot = 0; slot < MAX_PARTICLES; slot++) {
    if (g_particles[slot].active) {
      active++;
    }
  }
  return active;
}
