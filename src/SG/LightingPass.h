#pragma once

#include "SG/RenderPass.h"

namespace Parser { class Registry; }

namespace SG {

class Node;

/// LightingPass is a derived RenderPass that represents the final lighting
/// pass during multipass rendering.
class LightingPass : public RenderPass {
  public:
    virtual void Render(ion::gfx::Renderer &renderer, RenderData &data,
                        const FBTarget *fb_target) override;

  protected:
    virtual void AddFields() override;
    LightingPass() {}

  private:
    /// Sets Uniforms in the lighting shader in the given Node.
    void SetShaderUniforms_(RenderData &data, Node &node);

    friend class Parser::Registry;
};

}  // namespace SG
