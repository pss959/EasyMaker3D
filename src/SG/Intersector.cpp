#include "SG/Intersector.h"

#include <limits>
#include <stack>

#include <ion/math/matrixutils.h>
#include <ion/math/transformutils.h>

#include "Assert.h"
#include "Math/Intersection.h"
#include "Math/Linear.h"
#include "SG/Node.h"
#include "SG/NodePath.h"
#include "SG/Scene.h"
#include "SG/Shape.h"
#include "SG/Visitor.h"
#include "Util/KLog.h"

namespace SG {

// ----------------------------------------------------------------------------
// Intersector::Visitor_ class.
// ----------------------------------------------------------------------------

/// Derived Visitor class that does most of the intersection work.
class Intersector::Visitor_ : public Visitor {
  public:
    /// The constructor is passed the world-space ray to intersect.
    Visitor_(const Ray &world_ray) : world_ray_(world_ray) {
        // Start with an identity matrix.
        matrix_stack_.push(Matrix4f::Identity());
    }

    /// Returns the resulting Hit.
    const Hit & GetResultHit() const { return result_; }

  protected:
    virtual TraversalCode VisitNodeStart(const NodePath &path) override;
    virtual void            VisitNodeEnd(const NodePath &path) override;

  private:
    /// Ray to intersect, in world coordinates.
    const Ray world_ray_;

    /// Matrix stack for accumulating and restoring matrices. The current
    /// matrix is the top of the stack.
    std::stack<Matrix4f> matrix_stack_;

    /// Current shortest parametric distance to a Hit.
    float min_distance_ = std::numeric_limits<float>::max();

    /// Resulting Hit.
    Hit result_;
};

Visitor::TraversalCode Intersector::Visitor_::VisitNodeStart(
    const SG::NodePath &path) {
    const NodePtr &node = path.back();

    // Save and accumulate the model matrix. Do this before pruning because the
    // VisitNodeEnd() function always pops.
    ASSERTM(! matrix_stack_.empty(), "Cur path = " + path.ToString());
    Matrix4f cur_matrix = matrix_stack_.top() * node->GetModelMatrix();
    matrix_stack_.push(cur_matrix);

    // Skip this node or its entire subgraph if requested.
    if (! node->IsEnabled(Node::Flag::kTraversal) ||
        ! node->IsEnabled(Node::Flag::kIntersectAll))
        return TraversalCode::kPrune;
    if (! node->IsEnabled(Node::Flag::kIntersect))
        return TraversalCode::kContinue;

    // Transform the ray into the local coordinates of the node.
    Ray local_ray = TransformRay(world_ray_, ion::math::Inverse(cur_matrix));

    // If the ray does not intersect the local bounds of the node or it
    // intersects farther away, prune this subgraph. It is ok if the
    // intersection is exiting the box.
    float        distance;
    Bounds::Face face;
    bool         is_entry;
    const Bounds bounds = node->GetBounds();
    const bool hit_bounds =
        RayBoundsIntersectFace(local_ray, bounds, distance, face, is_entry);
    if (hit_bounds) {
        KLOG('i', Util::Spaces(2 * path.size())
             << "Intersected bounds of " << node->GetDesc()
             << " at " << distance);
        if (distance > min_distance_)
            return Visitor::TraversalCode::kPrune;
    }
    else {
        KLOG('i', Util::Spaces(2 * path.size())
             << "Missed bounds of " << node->GetDesc());
        return Visitor::TraversalCode::kPrune;
    }

    // Intersect each shape and get the closest intersection, if any.
    const auto &shapes = node->GetShapes();
    if (! shapes.empty()) {
        Hit shape_hit;
        shape_hit.path = path;
        for (const auto &shape: shapes) {
            // No good reason to check the bounds first, since most Nodes have
            // a single Shape and the bounds are the same.
            if (shape->IntersectRay(local_ray, shape_hit)) {
                KLOG('i', Util::Spaces(2 * path.size() + 1)
                     << "Intersected " << shape->GetDesc()
                     << " at " << shape_hit.distance);
                if (shape_hit.distance < min_distance_) {
                    min_distance_     = distance;
                    result_           = shape_hit;
                    result_.world_ray = world_ray_;
                    result_.shape     = shape;
                    result_.point     = world_ray_.GetPoint(distance);
                    result_.normal =
                        ion::math::Transpose(ion::math::Inverse(cur_matrix)) *
                        result_.normal;
                }
            }
            else {
                KLOG('i', Util::Spaces(2 * path.size() + 1)
                     << "Missed " << shape->GetDesc());
            }
        }
    }
    return Visitor::TraversalCode::kContinue;
}

void Intersector::Visitor_::VisitNodeEnd(const SG::NodePath &path) {
    // Restore the previous matrix.
    matrix_stack_.pop();
}

// ----------------------------------------------------------------------------
// Intersector functions.
// ----------------------------------------------------------------------------

Hit Intersector::IntersectScene(const Scene &scene, const Ray &ray) {
    Visitor_ visitor(ray);
    KLOG('i', "Intersecting scene");
    visitor.Visit(scene.GetRootNode());
    KLOG('i', "Intersection on " << visitor.GetResultHit().path.ToString());
    return visitor.GetResultHit();
}

}  // namespace SG
