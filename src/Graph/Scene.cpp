#include "Graph/Scene.h"

#include "Graph/Node.h"

namespace Graph {

Scene::Scene(const Util::FilePath &path) : path_(path), root_(new Node) {
}

void Scene::ResetCamera() {
    camera_ = Camera();
}

}  // namespace Graph
