#pragma once

#include "SG/RenderPass.h"

namespace SG {

//! LightingPass is a derived RenderPass that represents the final lighting
//! pass during multipass rendering.
class LightingPass : public RenderPass {
  public:
    virtual void AddFields() override;

    virtual void SetUpIon(IonContext &context) override;

    virtual void Render(ion::gfx::Renderer &renderer, PassData &data) override;
};

}  // namespace SG
