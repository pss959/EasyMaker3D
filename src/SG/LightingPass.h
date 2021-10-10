#pragma once

#include "SG/RenderPass.h"

namespace Parser { class Registry; }

namespace SG {

/// LightingPass is a derived RenderPass that represents the final lighting
/// pass during multipass rendering.
class LightingPass : public RenderPass {
  public:
    virtual void AddFields() override;
    virtual void SetUniforms(RenderData &data) override;
    virtual void Render(ion::gfx::Renderer &renderer, RenderData &data,
                        const FBTarget *fb_target) override;

  protected:
    LightingPass() {}

  private:
    /// Sets Uniforms in the named shader.
    void SetShaderUniforms_(RenderData &data, const std::string &shader_name);

    friend class Parser::Registry;
};

}  // namespace SG
