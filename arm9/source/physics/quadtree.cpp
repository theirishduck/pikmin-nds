#include "quadtree.h"
#include "body.h"
#include <algorithm>
#include "debug.h"

using physics::Body;
using numeric_types::fixed;
using numeric_types::literals::operator"" _f;

QuadTree::QuadTree(Vec2 lower_bounds, Vec2 upper_bounds) {
  lower_bounds_ = lower_bounds;
  upper_bounds_ = upper_bounds;
  max_radius_ = (upper_bounds.x - lower_bounds.x) / 2_f;
  if (max_radius_ < 0_f) {
    max_radius_ = max_radius_ * -1_f;
  }
}

QuadTree::~QuadTree() {
  DestroyChildren();
}

void QuadTree::DestroyChildren() {
  if (has_children_) {
    for (int x = 0; x < 2; x++) {
      for (int y = 0; y < 2; y++) {
        delete children_[x][y];
      }
    }
  }
  has_children_ = false;
}

void QuadTree::CreateChildren() {
  auto half_width = (upper_bounds_.x - lower_bounds_.x) / 2_f;
  auto half_height = (upper_bounds_.y - lower_bounds_.y) / 2_f;
  children_[0][0] = new QuadTree(
    Vec2{lower_bounds_.x, lower_bounds_.y},
    Vec2{upper_bounds_.x - half_width, upper_bounds_.y - half_height}
  );
  children_[0][1] = new QuadTree(
    Vec2{lower_bounds_.x + half_width, lower_bounds_.y},
    Vec2{upper_bounds_.x, upper_bounds_.y - half_height}
  );
  children_[1][0] = new QuadTree(
    Vec2{lower_bounds_.x, lower_bounds_.y + half_height},
    Vec2{upper_bounds_.x - half_width, upper_bounds_.y}
  );
  children_[1][1] = new QuadTree(
    Vec2{lower_bounds_.x + half_width, lower_bounds_.y + half_height},
    Vec2{upper_bounds_.x, upper_bounds_.y}
  );
  children_[0][0]->parent_ = this;
  children_[0][1]->parent_ = this;
  children_[1][0]->parent_ = this;
  children_[1][1]->parent_ = this;

  children_[0][0]->max_depth_ = max_depth_ - 1;
  children_[0][1]->max_depth_ = max_depth_ - 1;
  children_[1][0]->max_depth_ = max_depth_ - 1;
  children_[1][1]->max_depth_ = max_depth_ - 1;

  // Add our existing pool of objects to these children, if possible
  auto i = objects_.begin();
  while (i != objects_.end()) {
    if (AddObjectToChildren(*i)) {
      i = objects_.erase(i);
    } else {
      i++;
    }
  }

  has_children_ = true;
}

bool QuadTree::AddObjectToChildren(Body* object) {
  if (!has_children_) {
    return false;
  }
  // Short circuit: if this object is too big, we know it will
  // not fit inside any of our children, and we can bail early
  if (object->radius > max_radius_) {
    return false;
  }
  // Add this object to our child nodes, if possible.
  for (int x = 0; x < 2; x++) {
    for (int y = 0; y < 2; y++) {
      if (children_[x][y]->AddObject(object)) {
        return true;
      }
    }
  }
  return false;
}

bool QuadTree::AddObject(Body* object) {
  if (!has_children_) {
    if (AddObjectToChildren(object)) {
      return true;
    }
  }
  if (!parent_ or ObjectFitsInside(object)) {
    if (OwnObjectCount() < max_objects_ or max_depth_ <= 0) {
      if (object->is_pikmin) {
        pikmin_.push_back(object);
        object->current_tree = this;
      } else {
        objects_.push_back(object);
        object->current_tree = this;
      }
      return true; // It fits! I sits.
    }
    if (!has_children_) {
      // At this point, we know we have too many objects, so we need to
      // spawn in our 4 children
      CreateChildren();
    }
    if (AddObjectToChildren(object)) {
      return true;
    }
    // Add this object to ourselves, and set up references accordingly
    if (object->is_pikmin) {
      pikmin_.push_back(object);
      object->current_tree = this;
    } else {
      objects_.push_back(object);
      object->current_tree = this;
    }
    return true;
  }
  return false;
}

bool QuadTree::ObjectFitsInside(Body* object) {
  auto& pos = object->position;
  auto& radius = object->radius;
  auto object_lower_bounds = Vec2{pos.x - radius, pos.z - radius};
  auto object_upper_bounds = Vec2{pos.x + radius, pos.z + radius};
  if (
      object_lower_bounds.x >= lower_bounds_.x and
      object_lower_bounds.y >= lower_bounds_.y and
      object_upper_bounds.x < upper_bounds_.x and
      object_upper_bounds.y < upper_bounds_.y) {
    return true;
  }
  return false;
}

void QuadTree::UpdateObject(Body* object) {
  // We must assume the object has moved; let's handle everything that could
  // go wrong after that.
  if (parent_ and !ObjectFitsInside(object)) {
    // Move the object to our parent, then add it back in at the top of the
    // tree to bubble it down where it *does* fit.
    RemoveObject(object);
    Root()->AddObject(object);
    return;
  }
  if (has_children_) {
    // See if this object now fits into any of our children.
    if (AddObjectToChildren(object)) {
      // Remove it from Objects / Pikmin, if present
      auto index = std::find(objects_.begin(), objects_.end(), object);
      if (index != objects_.end()) {
        objects_.erase(index);
      }
      auto p_index = std::find(pikmin_.begin(), pikmin_.end(), object);
      if (p_index != pikmin_.end()) {
        pikmin_.erase(p_index);
      }
      return;
    }
  }
}

bool QuadTree::RemoveObject(Body* object) {
  // Are we in the objects list?
  auto index = std::find(objects_.begin(), objects_.end(), object);
  if (index != objects_.end()) {
    object->current_tree = nullptr;
    objects_.erase(index);
    if (OwnObjectCount() < max_objects_ and ChildObjectCount() == 0) {
      // We can safely despawn our children
      DestroyChildren();
    }
    return true;
  }

  // What about the pikmin list?
  auto p_index = std::find(pikmin_.begin(), pikmin_.end(), object);
  if (p_index != pikmin_.end()) {
    object->current_tree = nullptr;
    pikmin_.erase(p_index);
    if (OwnObjectCount() < max_objects_ and ChildObjectCount() == 0) {
      // We can safely despawn our children
      DestroyChildren();
    }
    return true;
  }

  // OK, how about any of our children? No?
  if (has_children_) {
    for (int x = 0; x < 2; x++) {
      for (int y = 0; y < 2; y++) {
        if (children_[x][y]->RemoveObject(object)) {
          if (OwnObjectCount() < max_objects_ and ChildObjectCount() == 0) {
            // We can safely despawn our children
            DestroyChildren();
          }
          return true;
        }
      }
    }
  }
  return false;
}

unsigned QuadTree::ChildObjectCount() {
  if (!has_children_) {
    return 0;
  }
  int count = 0;
  for (int x = 0; x < 2; x++) {
    for (int y = 0; y < 2; y++) {
      count += children_[x][y]->ObjectCount();
    }
  }
  return count;
}

unsigned int QuadTree::ObjectCount() {
  return ChildObjectCount() + OwnObjectCount();
}

unsigned int QuadTree::OwnObjectCount() {
  return pikmin_.size() + objects_.size();
}

std::vector<Body*>& QuadTree::Objects() {
  return objects_;
}

std::vector<Body*>& QuadTree::Pikmin() {
  return pikmin_;
}

QuadTree* QuadTree::Parent() {
  return parent_;
}

void QuadTree::DebugDraw() {
  debug::DrawLine(lower_bounds_, Vec2{upper_bounds_.x, lower_bounds_.y}, RGB5(0,31,31));
  debug::DrawLine(Vec2{upper_bounds_.x, lower_bounds_.y}, upper_bounds_, RGB5(0,31,31));
  debug::DrawLine(upper_bounds_, Vec2{lower_bounds_.x, upper_bounds_.y}, RGB5(0,31,31));
  debug::DrawLine(Vec2{lower_bounds_.x, upper_bounds_.y}, lower_bounds_, RGB5(0,31,31));
  if (has_children_) {
    for (int x = 0; x < 2; x++) {
      for (int y = 0; y < 2; y++) {
        children_[x][y]->DebugDraw();
      }
    }
  }

  // Display a red X for empty cells
  if (ObjectCount() <= 0) {
    debug::DrawLine(lower_bounds_, upper_bounds_, RGB5(31,10,10));
    debug::DrawLine(Vec2{upper_bounds_.x, lower_bounds_.y}, Vec2{lower_bounds_.x, upper_bounds_.y}, RGB5(31,10,10));
  }

  int object_count = objects_.size();
  fixed x_spacing = (upper_bounds_.x - lower_bounds_.x) / 5_f;
  fixed y_spacing = (upper_bounds_.y - lower_bounds_.y) / 5_f;
  fixed radius = (upper_bounds_.x - lower_bounds_.x) / 12_f;
  for (int i = 0; i < object_count and i < 16; i++) {
    int x = i % 4;
    int y = i / 4;
    fixed px = lower_bounds_.x + x_spacing + x_spacing * fixed::FromInt(x);
    fixed py = lower_bounds_.y + y_spacing + y_spacing * fixed::FromInt(y);
    debug::DrawCircle(Vec3{px, 0_f, py}, radius, RGB5(0,31,0), 4);
  }
}

QuadTree* QuadTree::Root() {
  QuadTree* root = this;
  while (root->parent_) {
    root = root->parent_;
  }
  return root;
}
