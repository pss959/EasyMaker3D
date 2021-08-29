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

//! This internal class maintains state during an intersection traversal.
struct Intersector_ {
    Ray      world_ray;
    Matrix4f cur_matrix        = Matrix4f::Identity();
    float    shortest_distance = std::numeric_limits<float>::max();
    Hit      result;

    Visitor::TraversalCode IntersectNode(const NodePath &cur_path) {
        ASSERT(! cur_path.empty());
        const NodePtr &node = cur_path.back();

        // Accumulate uModelMatrix.
        Matrix4f saved_matrix = cur_matrix;
        cur_matrix *= node->GetModelMatrix();

        // Transform the ray into the local coordinates of the node.
        Ray local_ray = TransformRay(world_ray, ion::math::Inverse(cur_matrix));

        // If the ray does not intersect the local bounds of the node or it
        // intersects farther away, prune this subgraph.
        float distance;
        if (! RayBoundsIntersect(local_ray, node->GetBounds(), distance) ||
            distance > shortest_distance)
            return Visitor::TraversalCode::kPrune;

        // Intersect each shape and get the closest intersection, if any.
        Hit shape_hit;
        shape_hit.path = cur_path;
        for (const auto &shape: node->GetShapes()) {
            // No good reason to check the bounds first, since most Nodes have
            // a single Shape and the bounds are the same.
            if (shape->IntersectRay(local_ray, shape_hit) &&
                shape_hit.distance < shortest_distance) {
                shortest_distance = distance;
                result = shape_hit;
                result.shape = shape;
            }
        }

        // Restore the previous matrix.
        cur_matrix = saved_matrix;

        return Visitor::TraversalCode::kContinue;
    }
};

Hit Intersector::IntersectScene(const Scene &scene, const Ray &ray) {
    Hit hit;

    Intersector_ helper;
    helper.world_ray = ray;

    Visitor visitor;
    visitor.Visit(scene.GetRootNode(),
                  std::bind(&Intersector_::IntersectNode, &helper,
                            std::placeholders::_1));
    return helper.result;
}

}  // namespace SG
