#pragma once

#include <vector>

#include "Parser/ObjectSpec.h"
#include "SG/Object.h"
#include "SG/Typedefs.h"
#include "Util/FilePath.h"

namespace SG {

//! A Scene object encapsulates a scene graph.
class Scene  : public Object {
  public:
    //! Returns the Camera for the scene.
    const CameraPtr & GetCamera() const { return camera_; }

    //! Returns the lights used for lighting the scene.
    const std::vector<PointLightPtr> & GetLights() const { return lights_; }

    //! Returns the render passes used to render the scene.
    const std::vector<RenderPassPtr> & GetRenderPasses() const {
        return render_passes_;
    }

    //! Sets the path the scene was read from, which can be helpful for
    //! messages and reloading.
    void SetPath(const Util::FilePath &path) { path_ = path; }

    //! Returns the path the scene was read from, if SetPath() was called.
    const Util::FilePath & GetPath() const { return path_; }

    //! Returns the root node of the last render pass, which is assumed to be
    //! the main lighting pass. Returns a null pointer if there are no render
    //! passes or the root node in the last one is null.
    NodePtr GetRootNode() const;

    // Applies a Visitor to call Update() for each Node.
    void Update() const;

    virtual void SetUpIon(IonContext &context) override;

    static Parser::ObjectSpec GetObjectSpec();

  private:
    //! \name Parsed Fields
    //!@{
    CameraPtr                  camera_;
    std::vector<PointLightPtr> lights_;
    std::vector<RenderPassPtr> render_passes_;
    //!@}

    //! Stores the path the scene was read from.
    Util::FilePath path_;
};

}  // namespace SG
