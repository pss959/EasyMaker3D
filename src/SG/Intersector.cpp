#include "SG/Intersector.h"

#include <limits>

#include <ion/math/matrixutils.h>

#include "Assert.h"
#include "SG/Node.h"
#include "SG/NodePath.h"
#include "SG/Scene.h"
#include "SG/Shape.h"
#include "SG/Visitor.h"

namespace SG {

// ----------------------------------------------------------------------------
// Intersector::Visitor_ class.
// ----------------------------------------------------------------------------

//! Derived Visitor class that does most of the intersection work.
class Intersector::Visitor_ : public Visitor {
  public:
    //! The constructor is passed the world-space ray to intersect.
    Visitor_(const Ray &world_ray) : world_ray_(world_ray) {}

    //! Returns the resulting Hit.
    const Hit & GetResultHit() const { return result_; }

  protected:
    virtual TraversalCode VisitNodeStart(const NodePath &path) override;
    virtual void            VisitNodeEnd(const NodePath &path) override;

  private:
    //! Ray to intersect, in world coordinates.
    const Ray world_ray_;

    //! Saved model matrix at the start of a Node visit.
    Matrix4f saved_matrix_;

    //! Current model matrix accumulated during traversal.
    Matrix4f cur_matrix_ = Matrix4f::Identity();

    //! Current shortest parametric distance to a Hit.
    float min_distance_ = std::numeric_limits<float>::max();

    //! Resulting Hit.
    Hit result_;
};

Visitor::TraversalCode Intersector::Visitor_::VisitNodeStart(
    const SG::NodePath &path) {
    const NodePtr &node = path.back();

    // Skip this node if requested.
    if (! node->IsEnabled(Node::Flag::kIntersect))
        return TraversalCode::kPrune;

    // Save and accumulate the model matrix.
    saved_matrix_ = cur_matrix_;
    cur_matrix_ *= node->GetModelMatrix();

    // Transform the ray into the local coordinates of the node.
    Ray local_ray = TransformRay(world_ray_, ion::math::Inverse(cur_matrix_));

    // If the ray does not intersect the local bounds of the node or it
    // intersects farther away, prune this subgraph. It is ok if the
    // intersection is exiting the box.
    float        distance;
    Bounds::Face face;
    bool         is_entry;
    const Bounds bounds = node->GetBounds();
    if (! RayBoundsIntersectFace(local_ray, bounds, distance, face, is_entry) ||
        distance > min_distance_)
        return Visitor::TraversalCode::kPrune;

    // Intersect each shape and get the closest intersection, if any.
    const auto &shapes = node->GetShapes();
    if (! shapes.empty()) {
        Hit shape_hit;
        shape_hit.path = path;
        for (const auto &shape: shapes) {
            // No good reason to check the bounds first, since most Nodes have
            // a single Shape and the bounds are the same.
            std::cerr << "XXXX Testing " << shape->GetTypeName()
                      << " in " << node->GetName() << "\n";
            if (shape->IntersectRay(local_ray, shape_hit) &&
                shape_hit.distance < min_distance_) {
                std::cerr << "XXXX Hit " << shape->GetTypeName()
                          << " at " << shape_hit.distance << "\n";
                min_distance_ = distance;
                result_ = shape_hit;
                result_.shape = shape;
            }
        }
    }
    return Visitor::TraversalCode::kContinue;
}

void Intersector::Visitor_::VisitNodeEnd(const SG::NodePath &path) {
    // Restore the previous matrix.
    cur_matrix_ = saved_matrix_;
}

// ----------------------------------------------------------------------------
// Intersector functions.
// ----------------------------------------------------------------------------

Hit Intersector::IntersectScene(const Scene &scene, const Ray &ray) {
    Visitor_ visitor(ray);
    visitor.Visit(scene.GetRootNode());
    return visitor.GetResultHit();
}

}  // namespace SG
