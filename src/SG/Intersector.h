#pragma once

#include <functional>

#include "Math/Types.h"
#include "SG/Hit.h"
#include "SG/Typedefs.h"

namespace SG {

//! The Intersector intersects a Ray with a scene graph to produce a Hit.
class Intersector {
  public:
    //! Intersects the given Scene with the given Ray, returning a Hit instance.
    static Hit IntersectScene(const Scene &scene, const Ray &ray);

  private:
    class Visitor_;  // Does most of the work.
};

}  // namespace SG
