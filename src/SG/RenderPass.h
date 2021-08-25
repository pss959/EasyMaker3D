#pragma once

#include <vector>

#include <ion/gfx/renderer.h>

#include "Interfaces/IRenderer.h"
#include "Parser/ObjectSpec.h"
#include "SG/Math.h"
#include "SG/Object.h"
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
            Point3f  position;                //!< From PointLight.
            Vector4f color;                   //!< From PointLight.
            Matrix4f bias_matrix;             //!< Computed by ShadowPass.
            Vector2f depth_range;             //!< Computed by ShadowPass.
            ion::gfx::TexturePtr shadow_map;  //!< Computed by ShadowPass.
        };

        Range2i                viewport;     //!< From View.
        Matrix4f               proj_matrix;  //!< Computed from View.
        Matrix4f               view_matrix;  //!< Computed from View.
        std::vector<LightData> per_light;    //!< LightData per light.

        //! Framebuffer target for rendering. XXXX Get rid of this?
        const IRenderer::FBTarget *fb_target = nullptr;
    };

    const NodePtr & GetRootNode() const { return root_; }

    //! Renders the pass using the given PassData and Ion renderer.
    virtual void Render(ion::gfx::Renderer &renderer, PassData &data) = 0;

    static Parser::ObjectSpec GetObjectSpec();

  protected:
    //! Convenience function that returns the Ion Node in the root node of the
    //! RenderPass. Asserts if there is none.
    const ion::gfx::NodePtr & GetIonRoot() const;

  private:
    //! \name Parsed Fields
    //!@{
    NodePtr root_;  //!< Root node to render.
    //!@}
};

}  // namespace SG
