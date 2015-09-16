#include "vector.h"
#include <vector>

namespace physics {
class Body;
}  // namespace physics

class QuadTree {
  public:
    QuadTree(Vec2 lower_bounds, Vec2 upper_bounds);
    ~QuadTree();
    bool AddObject(physics::Body* object);
    bool ObjectFitsInside(physics::Body* object);
    void UpdateObject(physics::Body* object);
    bool RemoveObject(physics::Body* object);
    unsigned int ObjectCount();
    std::vector<physics::Body*>& Objects();
    std::vector<physics::Body*>& Pikmin();
    QuadTree* Parent();
    Vec2 LowerBounds();
    Vec2 UpperBounds();
    void DebugDraw();
    QuadTree* Root();
  private:
    QuadTree* parent_{nullptr};
    QuadTree* children_[2][2];
    bool has_children_{false};
    unsigned int max_objects_{2};
    unsigned int max_depth_ = 5;
    std::vector<physics::Body*> objects_;
    std::vector<physics::Body*> pikmin_;
    Vec2 lower_bounds_;
    Vec2 upper_bounds_;
    numeric_types::fixed max_radius_;
    void CreateChildren();
    bool AddObjectToChildren(physics::Body* object);
    void DestroyChildren();
    unsigned int ChildObjectCount();
    unsigned int OwnObjectCount();
};
