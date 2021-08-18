#pragma once

#include "NParser/ObjectSpec.h"
#include "SG/Resource.h"
#include "SG/Typedefs.h"

namespace SG {

//! A Scene object encapsulates a scene graph.
class Scene : public Resource {
  public:
    //! Returns the Camera for the scene.
    const CameraPtr & GetCamera() const { return camera_; }

    //! Returns the root Node for the scene.
    const NodePtr & GetRootNode() const { return root_; }

    static NParser::ObjectSpec GetObjectSpec();

  private:
    // Parsed fields.
    CameraPtr camera_;  //! Camera for the scene.
    NodePtr   root_;    //! Root node of the scene.

    //! Redefines this to create a default Camera and empty root node if there
    //! is none.
    virtual void Finalize() override;
};

}  // namespace SG
