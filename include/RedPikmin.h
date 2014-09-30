#ifndef REDPIKMIN_H
#define REDPIKMIN_H

#include "MultipassEngine.h"

class RedPikmin : public DrawableEntity {
public:
    RedPikmin();
    ~RedPikmin();

    void update(MultipassEngine* engine) override;

private:
    bool NeedsNewTarget() const;
    void ChooseNewTarget();
    void Move();

    s16 rotation_ = 0;
    bool running_ = false;
    s32 updates_until_new_target_ = 0;
    Vec3 target_;
    Vec3 direction_;
};

#endif
