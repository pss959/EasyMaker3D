#pragma once

#include "NParser/FieldSpec.h"
#include "SG/Resource.h"
#include "SG/Typedefs.h"

namespace SG {

//! A Scene object encapsulates a scene graph.
class Scene : public Resource {
  public:
    //! The constructor is passed the FilePath the scene was read from.
    Scene(const Util::FilePath &path) : Resource(path) {}

    //! Returns the Camera for the scene.
    const CameraPtr & GetCamera() const { return camera_; }

    //! Returns the root Node for the scene.
    const NodePtr & GetRootNode() const { return root_; }

  private:
    CameraPtr camera_;  //! Camera for the scene.
    NodePtr   root_;    //! Root node of the scene.

    //! Redefines this to create a default Camera and empty root node if there
    //! is none.
    virtual void Finalize() override;

    static std::vector<NParser::FieldSpec> GetFieldSpecs_();
};

}  // namespace SG
