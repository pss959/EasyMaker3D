#pragma once

#include "SG/RenderPass.h"

namespace SG {

/// LightingPass is a derived RenderPass that represents the final lighting
/// pass during multipass rendering.
class LightingPass : public RenderPass {
  public:
    virtual void AddFields() override;
    virtual void SetUniforms(PassData &data) override;
    virtual void Render(ion::gfx::Renderer &renderer, PassData &data) override;

  private:
    /// Sets Uniforms in the named shader.
    void SetShaderUniforms_(PassData &data, const std::string &shader_name);
};

}  // namespace SG
