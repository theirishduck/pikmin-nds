#include "project_settings.h"
#include "multipass_engine.h"

#include <stdio.h>

#include <vector>

#include <nds/interrupts.h>
#include <nds/system.h>
#include <nds/arm9/background.h>
#include <nds/arm9/input.h>

#include "debug.h"

using namespace std;
using numeric_types::literals::operator"" _f;
using numeric_types::fixed;

using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;

using debug::Topic;

MultipassEngine::MultipassEngine() {
}

physics::World& MultipassEngine::World() {
  return world_;
}

void MultipassEngine::TargetEntity(DrawableEntity* entity) {
  camera_.FollowEntity(entity);
}

void MultipassEngine::AddEntity(DrawableEntity* entity) {
  entities_.push_back(entity);
  entity->set_engine(this);
  entity->Init();
}

void MultipassEngine::Update() {
  scanKeys();

  debug::StartTopic(Topic::kUpdate);
  for (auto entity : entities_) {
    entity->Update();
  }
  debug::EndTopic(Topic::kUpdate);

  debug::StartTopic(Topic::kPhysics);
  world_.Update();
  debug::EndTopic(Topic::kPhysics);

  camera_.Update();
  debug::UpdateInput();

  frame_counter_++;
}

int MultipassEngine::FrameCounter() {
  return frame_counter_;
}

Brads MultipassEngine::DPadDirection()  {
  // Todo(Nick) This feels messy. Find a way to make this cleaner.

  if (keysHeld() & KEY_RIGHT) {
    if (keysHeld() & KEY_UP) {
      return last_angle_ = 45_brad;
    }
    if (keysHeld() & KEY_DOWN) {
      return last_angle_ = 315_brad;
    }
    return last_angle_ = 0_brad;
  }

  if (keysHeld() & KEY_LEFT) {
    if (keysHeld() & KEY_UP) {
      return last_angle_ = 135_brad;
    }
    if (keysHeld() & KEY_DOWN) {
      return last_angle_ = 225_brad;
    }
    return last_angle_ = 180_brad;
  }

  if (keysHeld() & KEY_UP) {
    return last_angle_ = 90_brad;
  }

  if (keysHeld() & KEY_DOWN) {
    return last_angle_ = 270_brad;
  }

  return last_angle_;
}

Brads MultipassEngine::CameraAngle() {
  return camera_.GetAngle();
}

void ClipFriendlyPerspective(s32 near, s32 far, float angle) {
  // Setup a projection matrix that, critically, does not scale Z-values. This
  // ensures that no matter how the near and far plane are set, the resulting
  // z-coordinate is not stretched or squashed, and is more or less accurate.
  // (within rounding errors.) This is necessary for the clip planes to work
  // consistently between passes, at the cost of being slightly less accurate
  // when calculating depth values. (This is hardly noticable.)
  int ang = degreesToAngle(angle);
  int sine = sinLerp(ang);
  int cosine = sinLerp(ang);

  MATRIX_MULT4x4 = divf32((3 * cosine), (4 * sine));
  MATRIX_MULT4x4 = 0;
  MATRIX_MULT4x4 = 0;
  MATRIX_MULT4x4 = 0;

  MATRIX_MULT4x4 = 0;
  MATRIX_MULT4x4 = divf32(cosine, sine);
  MATRIX_MULT4x4 = 0;
  MATRIX_MULT4x4 = 0;

  MATRIX_MULT4x4 = 0;
  MATRIX_MULT4x4 = 0;
  MATRIX_MULT4x4 = -divf32(far + near, far - near);
  MATRIX_MULT4x4 = floattof32(-1.0);

  MATRIX_MULT4x4 = 0;
  MATRIX_MULT4x4 = 0;
  MATRIX_MULT4x4 = -divf32(2 * mulf32(far, near), far - near);
  MATRIX_MULT4x4 = 0;
}

void MultipassEngine::GatherDrawList() {
  // Set the projection matrix to a full frustrum so that the list can be sorted
  // without having to accout for errors caused by the clip plane.
  // 256 will be our backplane because it's a good largeish number which
  // reduces rouding errors.
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  ClipFriendlyPerspective(floattof32(0.1), floattof32(256.0), FIELD_OF_VIEW);
  glMatrixMode(GL_MODELVIEW);

  // Reset to the identity matrix in prep for calculations.
  glLoadIdentity();
  camera_.ApplyTransform();

  for (auto entity : entities_) {
    // Cache the object so its render information stays the same across
    // multiple passes.
    entity->SetCache();
    DrawState& state = entity->GetCachedState();

    // Using the camera state, calculate the nearest and farthest points,
    // which we'll later use to decide where the clipping planes should go.
    EntityContainer container;
    container.entity = entity;
    fixed object_z = entity->GetRealModelZ();
    container.far_z  = object_z + state.actor->Radius();
    container.near_z = object_z - state.actor->Radius();

    draw_list_.push(container);
  }
}

void MultipassEngine::ClearDrawList() {
  // Clear the draw list so that the next frame gets triggered.
  // It is emptied by looping because std::priority_queue does not provide
  // a clear function.
  while (not draw_list_.empty()) {
    draw_list_.pop();
  }
}

void MultipassEngine::SetVRAMforPass(int pass) {
  // VRAM banks A and B take turns being the display capture destination and the
  // texture used as the background for the next pass. The rear texture for the
  // first pass of each frame is not drawn because it is the start of a new
  // frame.
  if ((current_pass_ & 0x1) == 0) {
    vramSetBankA(VRAM_A_LCD);
    vramSetBankB(VRAM_B_TEXTURE_SLOT0);
    REG_DISPCAPCNT = DCAP_BANK(0) | DCAP_ENABLE | DCAP_SRC(1) | DCAP_SIZE(3);
  } else {
    vramSetBankA(VRAM_A_TEXTURE_SLOT0);
    vramSetBankB(VRAM_B_LCD);
    REG_DISPCAPCNT = DCAP_BANK(1) | DCAP_ENABLE | DCAP_SRC(1) | DCAP_SIZE(3);
  }

  // When the draw list has been emptied, the final pass has been reached. At
  // this point, the background that was showing is now set up for capture, and
  // the 3D engine is allowed to render directly to the screen. The result of
  // this pass is a complete frame, which is saved in VRAM bank D and then
  // displayed over the top of the next passes so that they aren't seen until
  // they are complete.
  if (draw_list_.empty()) {
    vramSetBankD(VRAM_D_LCD);
    videoSetMode(MODE_0_3D);
    REG_DISPCAPCNT = DCAP_BANK(3) | DCAP_ENABLE | DCAP_SRC(1) | DCAP_SIZE(3);
  } else {
    vramSetBankD(VRAM_D_MAIN_BG_0x06000000);
    videoSetMode(MODE_3_3D);
    bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    bgSetPriority(3, 0);
    bgSetPriority(0, 3);
  }
}

void MultipassEngine::DrawClearPlane() {
  if (current_pass_ == 0)
  {
    // Don't draw the rear-plane texture on the first pass; instead, the clear
    // color is visible.
    return;
  }

  // Because the rear texture needs to cover the whole screen no matter what,
  // draw it using an orthagonal projection.
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  ClipFriendlyPerspective(floattof32(0.1), floattof32(768.0), 1000.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Set the draw mode to quad, set up the texture format, and draw the back
  // plane.
  GFX_BEGIN = 1;
  u32 kTextureOffset{0};
  u32 kTextureWidth{5 << 20};  // 256 pixels
  u32 kTextureHeight{5 << 23};  // 256 pixels
  u32 kRepeatHorizontally{1 << 16};
  u32 kRepeatVertically{1 << 17};
  u32 kDirectTexture{7 << 26};
  GFX_TEX_FORMAT = kTextureOffset | kTextureWidth | kTextureHeight |
      kRepeatHorizontally | kRepeatVertically | kDirectTexture;

  // Draw the backplane as a white polygon so that it doesn't alter the captured
  // colors.
  // Todo(Nick) Investigate whether lighting should be disable when drawing the
  // back plane.
  glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK);
  glColor3b(255, 255, 255);

  glTranslatef(0.0, 0.0, -768.0);
  glScalef(1024.0, 768.0, 1.0);
  GFX_TEX_COORD = TEXTURE_PACK(inttot16(0), inttot16(0));
  glVertex3v16(floattov16(-1.0), floattov16(1.0), floattov16(0.0));

  GFX_TEX_COORD = TEXTURE_PACK(inttot16(0), inttot16(192));
  glVertex3v16(floattov16(-1.0), floattov16(-1.0), floattov16(0.0));

  GFX_TEX_COORD = TEXTURE_PACK(inttot16(256), inttot16(192));
  glVertex3v16(floattov16(1.0), floattov16(-1.0), floattov16(0.0));

  GFX_TEX_COORD = TEXTURE_PACK(inttot16(256), inttot16(0));
  glVertex3v16(floattov16(1.0), floattov16(1.0), floattov16(0.0));

  // Turn off textures for further polygons.
  GFX_TEX_FORMAT = 0;
}

void MultipassEngine::InitFrame() {
  //debug::TimingColor(RGB5(0, 15, 0));
  debug::StartTopic(Topic::kFrameInit);
  // Handle everything that happens at the start of a frame. This includes
  // gathering the initial draw list, and setting up caches for subsequent
  // passes.

  // Cache everything needed to draw this frame, as it may span multiple
  // passes and the state of these changing in the middle of a frame can cause
  // tearing.
  camera_.SetCache();
  GatherDrawList(); 

  // Ensure the overlap list is empty.
  overlap_list_.clear();
  current_pass_ = 0;

  // consoleClear();
  debug::EndTopic(Topic::kFrameInit);
}

void MultipassEngine::GatherPassList() {
  debug::StartTopic(Topic::kPassInit);

  // Build up the list of objects to render this pass.
  int polycount = 0;
  pass_list_.clear();

  // If there were any objects that straddle the current and previous passes,
  // ensure that they are drawn again this pass.
  // int overlaps_count = overlap_list_.size();
  for (auto entity : overlap_list_) {
    pass_list_.push_back(entity);
    polycount += pass_list_.back().entity->GetCachedState().actor->DrawCost();
  }
  overlap_list_.clear();

  // Pull entities from the list of all entities to draw this frame until all
  // objects are marked for drawing (marking a complete frame) or the polygon
  // quota is hit, whichever comes first.
  while (not draw_list_.empty() and polycount < MAX_POLYGONS_PER_PASS) {
    pass_list_.push_back(draw_list_.top());
    polycount += pass_list_.back().entity->GetCachedState().actor->DrawCost();
    draw_list_.pop();
  }

  debug::EndTopic(Topic::kPassInit);
}

bool MultipassEngine::ProgressMadeThisPass(unsigned int initial_length) {
  // If nothing was moved from the draw list for the frame this pass, than means
  //   1. There were no objects to draw at all this frame, or
  //   2. There is an object that exceeds the maximum polygon count per pass on
  //      its own, or there are too many objects in a perpendicular line to the
  //      camera's viewing angle.
  // Either way, there's nothing that can be done to fix it, so bail on this
  // frame and hope the next frame isn't as bad.
  // At some point in the future, it may be possible to change the level of
  // detail of some of the models to try to alleviate this problem.
  if (draw_list_.size() == initial_length) {
    if (not draw_list_.empty()) {
      printf("No progress made!\n");
      // TODO(Nick) Move the action for this check outside of this function;
      // it doesn't make sense for a simple check to have side effects.

      ClearDrawList();
    }

    GFX_FLUSH = 0;
    swiWaitForVBlank();
    return false;
  }
  return true;
}

void MultipassEngine::SetupDividingPlane() {
  // Now that the list of entities to render for this pass has been determined,
  // The near and far planes can be decided. For the first pass, the far plane
  // should be set as far back as possible, and for subsequent passes, the far
  // plane should be as far forward as the last pass reached forward.
  // The near plane should be at the front of the screen on the last pass, or
  // just behind the next entity to be drawn in the next pass.
  if (current_pass_ == 0) {
    far_plane_ = 256_f;
  } else {
    far_plane_ = near_plane_;
  }
  near_plane_ = 0.1_f;
  if (not draw_list_.empty()) {
    near_plane_ = draw_list_.top().far_z;
    // If that entity is too close to or behind the camera, then clamp the near
    // plane to just in front of the camera.
    if (near_plane_ < 0.1_f) {
      near_plane_ = 0.1_f;
    }
  }

  // Set up the matrices for the render based on the near and far plane
  // calculations.
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  ClipFriendlyPerspective(near_plane_.data_, far_plane_.data_, FIELD_OF_VIEW);
  printf("\x1b[%d;0H(%d)n: %.3f f: %.3f\n", current_pass_ + 1, current_pass_,
      (float)near_plane_, (float)far_plane_);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  camera_.ApplyTransform();
}

bool MultipassEngine::ValidateDividingPlane() {
  if (near_plane_ == far_plane_) {
    // One of two things has happened:
    //   1. Most likely, the front of the screen has been reached; i.e. both the
    //      near and far plane are 0.1_f. This is expected until some culling
    //      is implemented to remove all objects nearer than 0.1_f, and is
    //      perfectly safe to render.
    //   2. Lots of entities were piled on top of each other, and the engine was
    //      able to progress through the draw list some, but not enough to bring
    //      the far plane further forward. In this case, trying to continue to
    //      render will cause lots of overlapping/wrong depth order rendering,
    //      so prefer to drop a frame and hope the next frame has the entities
    //      spread a little further out.
    if (far_plane_ == 0.1_f) {
      printf("\x1b[10;0H Hit front of screen!\n");
      ClearDrawList();
      DrawClearPlane();

      GFX_FLUSH = 0;
      debug::StartTopic(Topic::kIdle);
      swiWaitForVBlank();
      debug::EndTopic(Topic::kIdle);

      SetVRAMforPass(current_pass_);
      current_pass_++;
    } else {
      printf("\x1b[10;0H Near/Far plane equal! BAD!\n");

      ClearDrawList();
      GFX_FLUSH = 0;
      swiWaitForVBlank();
    }
    return false;
  }
  return true;
}

void MultipassEngine::DrawPassList() {
  // Draw the entities for the pass.
  debug::StartTopic(Topic::kDraw);
  //int o = 0;
  for (auto& container : pass_list_) {
    //glLight(0, RGB15(31, 31, 31), floattov10(-0.40), floattov10(0.32), floattov10(0.27));
    //glLight(1, RGB15(31, 31, 31), floattov10(0.32), floattov10(0.32), floattov10(0.32));
    /*
    // TODO(Nick): Set this up to be turned on and off using a debug flag
    if (o++ < overlaps_count) {
      // Highlight the entities that are straddling passes by dimming them.
      glLight(0, RGB15(15, 15, 15), floattov10(-0.40), floattov10(0.32), floattov10(0.27));
      glLight(1, RGB15(15, 15, 15), floattov10(0.32), floattov10(0.32), floattov10(0.32));
    }//*/

    glPushMatrix();
    container.entity->Draw();
    glPopMatrix(1);

    // If this object is not fully drawn, add it to the overlap list to be
    // redrawn in the next pass.
    if (container.near_z < near_plane_ /*and near_plane_ > floattof32(0.1)*/) {
      overlap_list_.push_back(container);
    }
  }
  debug::EndTopic(Topic::kDraw);
}


void MultipassEngine::Draw() {
  if (draw_list_.empty()) {
    InitFrame();
  }

  unsigned int initial_length = draw_list_.size();
  GatherPassList();

  if (not ProgressMadeThisPass(initial_length)) {
    return;
  }

  SetupDividingPlane();

  if (not ValidateDividingPlane()) {
    return;
  }

  DrawPassList();

  // TODO(Nick): Turn the ground plane into an object
  // Draw the ground plane for debugging.
  // debug::DrawGroundPlane(64, 10, RGB5(0, 24 - current_pass_ * 6, 0));
  debug::DrawGroundPlane(64, 10, RGB5(0, 24, 0));

  // Todo(Cristian) Merge the logic of basic mechanics into the existing
  // framework and remove them from the repo.
  // void basicMechanicsDraw();
  // basicMechanicsDraw();

  if (debug::g_physics_circles) {
    world_.DebugCircles();
  }

  DrawClearPlane();

  GFX_FLUSH = 0;
  debug::StartTopic(Topic::kIdle);
  swiWaitForVBlank();


  if (debug::g_render_first_pass_only) {
    // Empty the draw list; limiting the frame to one pass.
    ClearDrawList();
  }

  SetVRAMforPass(current_pass_);
  current_pass_++;

  if (debug::g_skip_vblank) {
    // Spin wait until scanline 0 so that the timing colors are visible.
    while (REG_VCOUNT != 0) {
      continue;
    }
    irqEnable(IRQ_HBLANK);
    swiIntrWait(1, IRQ_HBLANK);
  }
  debug::EndTopic(Topic::kIdle);
}
