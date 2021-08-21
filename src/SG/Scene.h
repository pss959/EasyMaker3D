#pragma once

#include "Parser/ObjectSpec.h"
#include "SG/NodePath.h"
#include "SG/Object.h"
#include "SG/Typedefs.h"
#include "Util/FilePath.h"

namespace SG {

//! A Scene object encapsulates a scene graph.
class Scene  : public Object {
  public:
    //! Returns the Camera for the scene.
    const CameraPtr & GetCamera() const { return camera_; }

    //! Returns the root Node for the scene.
    const NodePtr & GetRootNode() const { return root_; }

    //! Sets the path the scene was read from, which can be helpful for
    //! messages and reloading.
    void SetPath(const Util::FilePath &path) { path_ = path; }

    //! Returns the path the scene was read from, if SetPath() was called.
    const Util::FilePath & GetPath() const { return path_; }

    virtual void SetUpIon(IonContext &context) override;

    static Parser::ObjectSpec GetObjectSpec();

  private:
    //! \name Parsed Fields
    //!@{
    CameraPtr camera_;  //! Camera for the scene.
    NodePtr   root_;    //! Root node of the scene.
    //!@}

    //! Stores the path the scene was read from.
    Util::FilePath path_;
};

}  // namespace SG
