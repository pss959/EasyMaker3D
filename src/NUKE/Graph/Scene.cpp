#include "Graph/Scene.h"

#include "Graph/Node.h"

namespace Graph {

void Scene::SetCamera_(const Camera camera) {
    camera_ = camera;
}

void Scene::SetRootNode_(const NodePtr &node) {
    root_ = node;
}

}  // namespace Graph
