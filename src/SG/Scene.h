#pragma once

#include "Parser/ObjectSpec.h"
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

    virtual void SetUpIon(IonContext &context) override;

    static Parser::ObjectSpec GetObjectSpec();

  private:
    // Parsed fields.
    CameraPtr camera_;  //! Camera for the scene.
    NodePtr   root_;    //! Root node of the scene.
};

}  // namespace SG
