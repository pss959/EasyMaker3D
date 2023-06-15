#include "SG/LightingPass.h"

#include <assert.h>

#include <ion/gfx/graphicsmanager.h>
#include <ion/gfx/node.h>
#include <ion/gfx/shaderinputregistry.h>
#include <ion/gfx/statetable.h>

#include "Base/FBTarget.h"
#include "SG/Node.h"
#include "SG/RenderData.h"
#include "SG/Search.h"
#include "SG/UniformBlock.h"
#include "Util/General.h"

namespace SG {

void LightingPass::Render(ion::gfx::Renderer &renderer, RenderData &data,
                          const FBTarget *fb_target) {
    // Find each Node that uses either of the shaders that need to be set up.
    auto match = [](const SG::Node &node){
        return ! node.GetShaderNames().empty() &&
            Util::Contains(node.GetShaderNames(), "Lighting");
    };
    const std::vector<SG::NodePtr> nodes =
        SG::FindUniqueNodes(data.root_node, match);
    ASSERT(nodes.size() == 1U);
    for (auto &node: nodes)
        SetShaderUniforms_(data, *node);

    // Set the viewport in the root StateTable to the window size.
    ASSERT(data.root_node);
    const ion::gfx::NodePtr ion_root = data.root_node->GetIonNode();
    ASSERT(ion_root);
    ASSERT(ion_root->GetStateTable());
    ion_root->GetStateTable()->SetViewport(data.viewport);

    // Set up the framebuffer(s).
    ion::gfx::GraphicsManager &gm = *renderer.GetGraphicsManager();
    if (fb_target) {
        ASSERT(fb_target->rendered_fbo.Get());
        ASSERT(fb_target->resolved_fbo.Get());
        renderer.BindFramebuffer(fb_target->rendered_fbo);
    }
    else {
        renderer.BindFramebuffer(ion::gfx::FramebufferObjectPtr());
    }

    // This is required in VR to keep the controller geometry from being
    // clipped by the near plane.
    gm.Enable(GL_DEPTH_CLAMP);

    renderer.DrawScene(ion_root);

    if (fb_target) {
        const uint32 kMask =
            ion::gfx::Renderer::kColorBufferBit |
            ion::gfx::Renderer::kDepthBufferBit;
        renderer.ResolveMultisampleFramebuffer(fb_target->rendered_fbo,
                                               fb_target->resolved_fbo, kMask);
    }
}

void LightingPass::SetShaderUniforms_(RenderData &data, Node &node) {
    auto &block = node.GetUniformBlockForPass(GetName());
    ASSERT(block.GetIonUniformBlock());
    auto &ion_block = *block.GetIonUniformBlock();

    // Set pass-independent uniforms.
    block.SetModelMatrices(Matrix4f::Identity(), data.view_matrix);

    // Set global pass-specific uniforms.
    const int light_count = static_cast<int>(data.per_light.size());
    SetIonUniform(ion_block, "uViewportSize",     data.viewport.GetSize());
    SetIonUniform(ion_block, "uProjectionMatrix", data.proj_matrix);
    SetIonUniform(ion_block, "uViewMatrix",       data.view_matrix);
    SetIonUniform(ion_block, "uViewPos",          data.view_pos);
    SetIonUniform(ion_block, "uLightCount",       light_count);

    // Set per-light uniforms.
    for (int i = 0; i < light_count; ++i) {
        RenderData::LightData &ldata = data.per_light[i];
        SetIonUniformAt(ion_block, "uLightPos",       i, ldata.position);
        SetIonUniformAt(ion_block, "uLightColor",     i, ldata.color);
        SetIonUniformAt(ion_block, "uLightMatrix",    i, ldata.light_matrix);
        SetIonUniformAt(ion_block, "uLightShadowMap", i, ldata.shadow_map);
    }
}

}  // namespace SG
