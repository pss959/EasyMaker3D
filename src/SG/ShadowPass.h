//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <vector>

#include <ion/gfx/framebufferobject.h>
#include <ion/gfx/texture.h>

#include "SG/RenderData.h"
#include "SG/RenderPass.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(ShadowPass);

/// ShadowPass is a derived RenderPass that represents a shadow pass during
/// multipass rendering. It actually does one render pass per light.
///
/// \ingroup SG
class ShadowPass : public RenderPass {
  public:
    /// Sets a flag indicating whether shadows are enabled for the scene. The
    /// default is true.
    void SetShadowsEnabled(bool enabled) { are_shadows_enabled_ = enabled; }

    /// Returns a flag indicating whether shadows are enabled for the
    /// scene. The default is true.
    bool AreShadowsEnabled() const { return are_shadows_enabled_; }

    virtual void Render(ion::gfx::Renderer &renderer, RenderData &data,
                        const FBTarget *fb_target) override;

  protected:
    ShadowPass() {}

  private:
    /// Struct storing items needed for each light.
    struct PerLight_ {
        /// Texture that holds the shadow depth map.
        ion::gfx::TexturePtr           texture;
        /// FramebufferObject that holds the result of rendering the depth.
        ion::gfx::FramebufferObjectPtr fbo;
    };

    /// Per-light Ion infrastructure for creating shadow maps.
    std::vector<PerLight_> per_light_;

    /// This lets the ShadowPass know if it has set up global uniforms.
    bool were_uniforms_created_ = false;

    /// Whether shadows are enabled.
    bool are_shadows_enabled_ = true;

    /// Creates the PerLight_ data for the indexed light in the RenderData.
    void CreatePerLightData_(RenderData &data, size_t index);

    /// Updates some fields in the RenderData::LightData instance.
    void SetPerLightData_(const PerLight_ &pldata, RenderData::LightData &data);

    /// Sets Uniforms in the shader in the given Node.
    void SetShaderUniforms_(RenderData &data, Node &node);

    friend class Parser::Registry;
};

}  // namespace SG
