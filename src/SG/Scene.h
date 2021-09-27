#pragma once

#include <vector>

#include "SG/Gantry.h"
#include "SG/Object.h"
#include "SG/PointLight.h"
#include "SG/RenderPass.h"
#include "SG/Typedefs.h"
#include "Util/FilePath.h"

namespace Parser { class Registry; }

namespace SG {

/// A Scene object encapsulates a scene graph.
class Scene  : public Object {
  public:
    virtual void AddFields() override;

    /// Redefines this to check the log_key_string_ field and set up logging if
    /// it is set.
    virtual void SetFieldParsed(const Parser::Field &field) override;

    /// Returns the camera gantry for the scene.
    const GantryPtr & GetGantry() const { return gantry_; }

    /// Returns the lights used for lighting the scene.
    const std::vector<PointLightPtr> & GetLights() const { return lights_; }

    /// Returns the render passes used to render the scene.
    const std::vector<RenderPassPtr> & GetRenderPasses() const {
        return render_passes_;
    }

    /// Sets the path the scene was read from, which can be helpful for
    /// messages and reloading.
    void SetPath(const Util::FilePath &path) { path_ = path; }

    /// Returns the path the scene was read from, if SetPath() was called.
    const Util::FilePath & GetPath() const { return path_; }

    /// Returns the root node of the last render pass, which is assumed to be
    /// the main lighting pass. Returns a null pointer if there are no render
    /// passes or the root node in the last one is null.
    NodePtr GetRootNode() const;

  protected:
    Scene() {}

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string>         log_key_string_{"log_key_string"};
    Parser::ObjectField<Gantry>         gantry_{"gantry"};
    Parser::ObjectListField<PointLight> lights_{"lights"};
    Parser::ObjectListField<RenderPass> render_passes_{"render_passes"};
    ///@}

    /// Stores the path the scene was read from.
    Util::FilePath path_;

    friend class Parser::Registry;
};

}  // namespace SG
