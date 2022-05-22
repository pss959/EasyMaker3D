#pragma once

#include <vector>

#include "SG/ColorMap.h"
#include "SG/Gantry.h"
#include "SG/IonContext.h"
#include "SG/Object.h"
#include "SG/Node.h"
#include "SG/PointLight.h"
#include "SG/RenderPass.h"
#include "SG/UniformBlock.h"
#include "Util/FilePath.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(Scene);

/// A Scene object encapsulates a scene graph.
///
/// \ingroup SG
class Scene  : public Object {
  public:
    /// Scenes are scoped.
    virtual bool IsScoped() const override { return true; }

    /// Redefines this to check the log_key_string_ field and set up logging if
    /// it is set.
    virtual void SetFieldParsed(const Parser::Field &field) override;

    /// Returns the ColorMap containing defining named colors for the scene.
    const ColorMapPtr & GetColorMap() const { return color_map_; }

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
    void SetPath(const FilePath &path) { path_ = path; }

    /// Returns the path the scene was read from, if SetPath() was called.
    const FilePath & GetPath() const { return path_; }

    /// Returns the root node of the scene. May be null.
    const NodePtr & GetRootNode() const { return root_node_; }

    /// Sets up all of the Ion data in the Scene using the given IonContext,
    /// which is updated by the Scene and stored in all Nodes in the scene.
    void SetUpIon(const IonContextPtr &ion_context);

  protected:
    Scene() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string>         log_key_string_{"log_key_string"};
    Parser::ObjectField<ColorMap>       color_map_{"color_map"};
    Parser::ObjectField<Gantry>         gantry_{"gantry"};
    Parser::ObjectListField<PointLight> lights_{"lights"};
    Parser::ObjectListField<RenderPass> render_passes_{"render_passes"};
    Parser::ObjectField<Node>           root_node_{"root_node"};
    ///@}

    /// Stores the path the scene was read from.
    FilePath path_;

    friend class Parser::Registry;
};

}  // namespace SG
