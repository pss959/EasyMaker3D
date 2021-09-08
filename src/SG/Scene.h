#pragma once

#include <vector>

#include "SG/Camera.h"
#include "SG/Object.h"
#include "SG/PointLight.h"
#include "SG/RenderPass.h"
#include "SG/Typedefs.h"
#include "Util/FilePath.h"

namespace SG {

//! A Scene object encapsulates a scene graph.
class Scene  : public Object {
  public:
    virtual void AddFields() override;

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

    virtual void SetUpIon(const ContextPtr &context) override;

  private:
    class Updater_;

    //! \name Parsed Fields
    //!@{
    Parser::ObjectField<Camera>         camera_{"camera"};
    Parser::ObjectListField<PointLight> lights_{"lights"};
    Parser::ObjectListField<RenderPass> render_passes_{"render_passes"};
    //!@}

    //! Stores the path the scene was read from.
    Util::FilePath path_;
};

}  // namespace SG
