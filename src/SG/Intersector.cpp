#include "SG/Intersector.h"

#include <limits>
#include <stack>

#include <ion/math/matrixutils.h>
#include <ion/math/transformutils.h>

#include "Math/Intersection.h"
#include "Math/Linear.h"
#include "SG/Node.h"
#include "SG/NodePath.h"
#include "SG/Scene.h"
#include "SG/Shape.h"
#include "Util/Assert.h"
#include "Util/KLog.h"

namespace SG {

// ----------------------------------------------------------------------------
// Intersector::Visitor_ class.
// ----------------------------------------------------------------------------

/// The Intersector::Visitor_ class does most of the work by traversing the
/// graph and intersecting the ray with the contents. It checks bounds first at
/// every Node before intersecting shapes.
class Intersector::Visitor_  {
  public:
    /// Intersects the given ray (in world coordinates) with the the scene
    /// rooted by the given Node.  Returns the resulting Hit.
    Hit IntersectScene(const Ray &world_ray, const NodePtr &root);

  private:
    /// Current shortest parametric distance to a Hit.
    float min_distance_ = std::numeric_limits<float>::max();

    /// Resulting Hit.
    Hit   result_hit_;

    /// Intersects the given ray (in world coordinates) with the subgraph
    /// rooted by the Node at the tail of the given NodePath. The current model
    /// matrix is provided.  Stores the closest Hit, if any, in result_hit_.
    void IntersectSubgraph_(const Ray &world_ray, const NodePath &path,
                            const Matrix4f &matrix);

    /// If the given ray (in object coordinates) intersects the bounds of the
    /// Node at the tail of the given path before the current min_distance_,
    /// this returns true and sets distance to the new distance. Otherwise, it
    /// just returns false.
    bool IntersectNodeBounds_(const Ray &obj_ray, const NodePath &path,
                              float &distance);

    /// Intersects the object-coordinate ray with all Shapes in the Node at the
    /// tail of the given NodePath. Stores the closest Hit, if any, in
    /// result_hit_. The world ray, path, and current matrix are provided to
    /// set up the Hit. Returns true if any Shape was hit.
    bool IntersectShapes_(const Ray &world_ray, const Ray &obj_ray,
                          const NodePath &path, const Matrix4f &matrix);
};

Hit Intersector::Visitor_::IntersectScene(const Ray &world_ray,
                                          const NodePtr &root) {
    if (root)
        IntersectSubgraph_(world_ray, NodePath(root), Matrix4f::Identity());
    return result_hit_;
}

void Intersector::Visitor_::IntersectSubgraph_(const Ray &world_ray,
                                               const NodePath &path,
                                               const Matrix4f &matrix) {
    const NodePtr &node = path.back();

    // Skip the entire subgraph if either of these flags is set.
    if (! node->IsEnabled(Node::Flag::kTraversal) ||
        ! node->IsEnabled(Node::Flag::kIntersectAll))
        return;

    // Accumulate the matrix for this node.
    const Matrix4f cur_matrix = matrix * node->GetModelMatrix();

    // Transform the ray into object coordinates for this Node.
    const Ray obj_ray = TransformRay(world_ray, ion::math::Inverse(cur_matrix));

    // If the ray misses the Node's bounds or hits it past the current
    // intersection distance, skip the subgraph.
    float distance;
    if (IntersectNodeBounds_(obj_ray, path, distance)) {
        // If the node is using its bounds as an intersection proxy, return the
        // hit on the bounds. Since this should never be used for exact
        // intersections, the shape, point, and normal do not really matter.
        if (node->ShouldUseBoundsProxy()) {
            result_hit_.path         = path;
            result_hit_.world_ray    = world_ray;
            result_hit_.distance     = distance;
            result_hit_.point        = obj_ray.GetPoint(distance);
            result_hit_.normal       = Vector3f(0, 0, 1);     // Does not matter.
            result_hit_.bounds_point = result_hit_.point;
        }

        // Intersect with shapes in this Node but only if the kIntersect flag
        // is enabled. If a shape is intersected, store the bounds intersection
        // point.
        if (node->IsEnabled(Node::Flag::kIntersect) &&
            IntersectShapes_(world_ray, obj_ray, path, cur_matrix)) {
            result_hit_.bounds_point = obj_ray.GetPoint(distance);
        }

        // Continue to children.
        for (const auto &child: node->GetAllChildren()) {
            NodePath child_path = path;
            child_path.push_back(child);
            IntersectSubgraph_(world_ray, child_path, cur_matrix);
        }
    }
}

bool Intersector::Visitor_::IntersectNodeBounds_(const Ray &obj_ray,
                                                 const NodePath &path,
                                                 float &distance) {
    const NodePtr &node = path.back();
    const Bounds bounds = node->GetBounds();

    // Note that it is ok if the only intersection is exiting the box.
    Bounds::Face face;
    bool         is_entry;
    if (RayBoundsIntersectFace(obj_ray, bounds, distance, face, is_entry)) {
        KLOG('i', Util::Spaces(2 * path.size())
             << "Hit bounds of " << node->GetDesc()
             << " at " << distance << " z=" << obj_ray.GetPoint(distance)[2]);
        if (distance > min_distance_)
            return false;
    }
    else {
        KLOG('i', Util::Spaces(2 * path.size())
             << "Missed bounds of " << node->GetDesc());
        return false;
    }
    return true;
}

bool Intersector::Visitor_::IntersectShapes_(const Ray &world_ray,
                                             const Ray &obj_ray,
                                             const NodePath &path,
                                             const Matrix4f &matrix) {
    // Intersect each shape and get the closest intersection, if any.
    const NodePtr &node = path.back();
    const auto &shapes = node->GetShapes();
    bool any_hit = false;
    if (! shapes.empty()) {
        Hit shape_hit;
        shape_hit.path = path;
        for (const auto &shape: shapes) {
            // No good reason to check the bounds first, since most Nodes have
            // a single Shape and the bounds are the same.
            if (shape->IntersectRay(obj_ray, shape_hit)) {
                KLOG('i', Util::Spaces(2 * path.size() + 1)
                     << "Intersected " << shape->GetDesc()
                     << " at " << shape_hit.distance);
                const float dist = shape_hit.distance;
                if (dist < min_distance_) {
                    min_distance_         = dist;
                    result_hit_           = shape_hit;
                    result_hit_.world_ray = world_ray;
                    result_hit_.shape     = shape;
                    result_hit_.point     = obj_ray.GetPoint(dist);
                    result_hit_.normal    = shape_hit.normal;
                }
                any_hit = true;
            }
            else {
                KLOG('i', Util::Spaces(2 * path.size() + 1)
                     << "Missed " << shape->GetDesc());
            }
        }
    }
    return any_hit;
}

// ----------------------------------------------------------------------------
// Intersector functions.
// ----------------------------------------------------------------------------

Hit Intersector::IntersectScene(const Scene &scene, const Ray &ray) {
    Visitor_ visitor;
    KLOG('i', "Intersecting scene");
    const Hit hit = visitor.IntersectScene(ray, scene.GetRootNode());
    KLOG('i', "Intersection on " << hit.path.ToString());
    return hit;
}

}  // namespace SG
