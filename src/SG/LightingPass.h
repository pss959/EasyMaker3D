//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "SG/RenderPass.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

namespace SG {

class Node;
DECL_SHARED_PTR(LightingPass);

/// LightingPass is a derived RenderPass that represents the final lighting
/// pass during multipass rendering.
///
/// \ingroup SG
class LightingPass : public RenderPass {
  public:
    virtual void Render(ion::gfx::Renderer &renderer, RenderData &data,
                        const FBTarget *fb_target) override;

  protected:
    LightingPass() {}

  private:
    /// Sets Uniforms in the lighting shader in the given Node.
    void SetShaderUniforms_(RenderData &data, Node &node);

    friend class Parser::Registry;
};

}  // namespace SG
