//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <functional>

#include "Math/Types.h"
#include "SG/Hit.h"

namespace SG {

class Scene;
DECL_SHARED_PTR(Node);

/// The Intersector intersects a Ray with a scene graph to produce a Hit.
///
/// \ingroup SG
class Intersector {
  public:
    /// Intersects the given Scene with the given Ray, returning a Hit instance.
    static Hit IntersectScene(const Scene &scene, const Ray &ray);

    /// Intersects the subgraph rooted by the given Node with the given Ray,
    /// returning a Hit instance. If \p bounds_only is true, only Node bounds
    /// are intersected instead of their meshes.
    static Hit IntersectGraph(const SG::NodePtr &root, const Ray &ray,
                              bool bounds_only = false);

  private:
    class Visitor_;  // Does most of the work.
};

}  // namespace SG
