#pragma once

#include "NParser/FieldSpec.h"
#include "SG/Resource.h"
#include "SG/Typedefs.h"

namespace SG {

//! A Scene object encapsulates a scene graph.
class Scene : public Resource {
  public:
    //! Returns the Camera stored in the scene.
    const CameraPtr & GetCamera() const { return camera_; }

    //! Returns the root Node for the scene.
    const NodePtr & GetRootNode() const { return root_; }

  private:
    CameraPtr camera_;  //! Camera for the scene.
    NodePtr   root_;    //! Root node of the scene.

    static std::vector<NParser::FieldSpec> GetFieldSpecs_();
};

}  // namespace SG
