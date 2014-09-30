#ifndef MULTIPASSENGINE_H
#define MULTIPASSENGINE_H

#include <queue>
#include "drawable_entity.h"
#include "project_settings.h"

struct EntityContainer {
    DrawableEntity* entity;
    gx::Fixed<s32,12> near_z;
    gx::Fixed<s32,12> far_z;
    bool operator< (const EntityContainer& other) const {return far_z <  other.far_z;}
        
};

class MultipassEngine {
    private:
        std::priority_queue<EntityContainer> drawList;
        
        std::vector<DrawableEntity*> entities;
        std::vector<EntityContainer> overlap_list;
        std::vector<EntityContainer> pass_list;
        
        int current_pass = 0;

        bool debug_first_pass = false;
        bool debug_timings = false;
        bool debug_colors = false;
        
        void gatherDrawList();
        void setVRAMforPass(int pass);
        void applyCameraTransform();
        void drawClearPlane();

        int old_keys;
        int keys;
        int last_angle = 0;

        gx::Fixed<s32,12> near_plane;
        gx::Fixed<s32,12> far_plane;

        Vec3 camera_position_current;
        Vec3 camera_target_current;

        Vec3 camera_position_destination;
        Vec3 camera_target_destination;

        Vec3 camera_position_cached;
        Vec3 camera_target_cached;

        DrawableEntity* entity_to_follow;

        bool highCamera = false;
        int cameraDistance = 2;

    public:
        MultipassEngine();
        void drawEntity(DrawableEntity entity);
        
        //called during gameloop to run the engine
        void update();
        void draw();
        
        void addEntity(DrawableEntity* entity);
        
        static MultipassEngine* engine;

        int dPadDirection();
        int cameraAngle();

        void updateCamera();
        void setCamera(Vec3 position, Vec3 target);
        void targetEntity(DrawableEntity*);
};

#endif