#pragma once

#include <functional>

#include "SG/Hit.h"
#include "SG/Math.h"
#include "SG/Typedefs.h"

namespace SG {

//! The Intersector intersects a Ray with a scene graph to produce a Hit.
class Intersector {
  public:
    //! Intersects the given Scene with the given Ray, returning a Hit instance.
    static Hit IntersectScene(const Scene &scene, const Ray &ray);
};

}  // namespace SG
