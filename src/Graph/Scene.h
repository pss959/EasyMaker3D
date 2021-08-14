#pragma once

#include "Graph/Camera.h"
#include "Graph/Object.h"
#include "Graph/Typedefs.h"
#include "Util/FilePath.h"

namespace Graph {

//! A Scene object encapsulates a scene graph.
class Scene : public Object {
  public:
    //! The constructor is passed the path that the scene is read from.
    Scene(const Util::FilePath &path);
    ~Scene();

    //! Returns the path the scene was read from.
    const Util::FilePath & GetPath() const { return path_; }

    //! Returns the Camera read with the scene.
    const Camera & GetCamera() const { return camera_; }

    //! Returns the root Node for the scene. This should remain constant across
    //! reloads of the scene from its file.
    const NodePtr & GetRootNode() const { return root_; }

    //! Resets the Camera in the scene to default values.
    void ResetCamera();

  private:
    const Util::FilePath path_;    //! Path to the scene's file.
    Camera               camera_;  //! Camera for the scene.
    NodePtr              root_;    //! Root node of the scene.

    //! Stores a new Camera in the scene.
    void SetCamera_(const Camera camera) { camera_ = camera; }

    friend class Reader_;
};

}  // namespace Graph
