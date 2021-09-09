#pragma once

#include <vector>

#include <ion/gfx/renderer.h>

#include "Interfaces/IRenderer.h"
#include "Math/Types.h"
#include "SG/Node.h"
#include "SG/PassType.h"
#include "SG/Typedefs.h"

class View;

namespace SG {

//! RenderPass is a base class for an object representing one pass during
//! multipass rendering.
class RenderPass : public Object {
  public:
    //! This struct encapsulates all of the data used by derived classes to
    //! render.
    struct PassData {
        //! Per-light data for rendering.
        struct LightData {
            Point3f              position;       //!< From light (world coords).
            Color                color;          //!< From light.
            bool                 casts_shadows;  //!< From light.
            Matrix4f             light_matrix;   //!< Computed by ShadowPass.
            ion::gfx::TexturePtr shadow_map;     //!< Generated by ShadowPass.
        };

        Range2i                viewport;         //!< From View.
        Matrix4f               proj_matrix;      //!< Computed from View.
        Matrix4f               view_matrix;      //!< Computed from View.
        Point3f                view_pos;         //!< Set from View.
        std::vector<LightData> per_light;        //!< LightData per light.

        //! Framebuffer target for rendering. XXXX Get rid of this?
        const IRenderer::FBTarget *fb_target = nullptr;
    };

    virtual void AddFields() override;

    // Returns the specific PassType for this instance.
    virtual PassType GetPassType() const = 0;

    const NodePtr & GetRootNode() const { return root_; }

    // Updates the RenderPass's scene graph for rendering. The main effect of
    // doing this is to allow UniformBlock instances to be enabled or disabled
    // depending on the name of the pass.
    void UpdateForRender();

    //! Renders the pass using the given PassData and Ion renderer.
    virtual void Render(ion::gfx::Renderer &renderer, PassData &data) = 0;

  protected:
    //! Convenience function that returns the Ion Node in the root node of the
    //! RenderPass. Asserts if there is none.
    const ion::gfx::NodePtr & GetIonRoot() const;

  private:
    class Updater_;

    //! \name Parsed Fields
    //!@{
    Parser::ObjectField<Node> root_{"root"};
    //!@}
};

}  // namespace SG
