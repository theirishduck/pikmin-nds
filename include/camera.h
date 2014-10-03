#ifndef CAMERA_H
#define CAMERA_H

#include "drawable_entity.h"

class Camera {
	private:
		DrawableEntity* target_;

		Vec3 position_current_;
        Vec3 target_current_;

        Vec3 position_destination_;
        Vec3 target_destination_;

        Vec3 position_cached_;
        Vec3 target_cached_;

        DrawableEntity* entity_to_follow_;

        bool high_camera_{false};
        int distance_{2};

	public:
		Camera();
        void setCamera(Vec3 position, Vec3 target, bool instant = false);
        void targetEntity(DrawableEntity* target);

        int getAngle();

		void update();
        void applyTransform();
        void setCache();
};

#endif