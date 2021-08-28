#include "SG/Intersector.h"

#include <limits>

#include "SG/Node.h"
#include "SG/NodePath.h"
#include "SG/Scene.h"
#include "SG/Visitor.h"

namespace SG {

//! This internal class maintains state during an intersection traversal.
struct Intersector_ {
    NodePath cur_path;
    Matrix4f cur_matrix        = Matrix4f::Identity();
    float    shortest_distance = std::numeric_limits<float>::max();
    Hit      result;

    Visitor::TraversalCode IntersectNode(const NodePtr &node) {
        // Accumulate uModelMatrix.
        Matrix4f saved_matrix = cur_matrix;
        cur_matrix *= node->GetModelMatrix();

#if XXXX
        for (const auto &shape: node->GetShapes()) {
            // XXXX Intersect the shape...
            // if (XXXX) {
            // }
        }
#endif

        cur_matrix = saved_matrix;

        return Visitor::TraversalCode::kContinue;
    }
};

Hit Intersector::IntersectScene(const Scene &scene, const Ray &ray) {
    Hit hit;

    Intersector_ helper;

    Visitor visitor;
    visitor.Visit(scene.GetRootNode(),
                  std::bind(&Intersector_::IntersectNode, &helper,
                            std::placeholders::_1));
    return helper.result;
}

}  // namespace SG
