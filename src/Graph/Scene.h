#pragma once

#include "Graph/Camera.h"
#include "Graph/Resource.h"
#include "Graph/Typedefs.h"

namespace Input { class Extractor; }

namespace Graph {

//! A Scene object encapsulates a scene graph.
class Scene : public Resource {
  public:
    //! Returns the Camera stored in the scene.
    const Camera & GetCamera() const { return camera_; }

    //! Returns the root Node for the scene.
    const NodePtr & GetRootNode() const { return root_; }

  private:
    Camera  camera_;  //! Camera for the scene.
    NodePtr root_;    //! Root node of the scene.

    //! Stores a new Camera in the scene.
    void SetCamera_(const Camera camera) { camera_ = camera; }

    friend class ::Input::Extractor;
};

}  // namespace Graph
