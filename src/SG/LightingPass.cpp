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

namespace SG {

void LightingPass::AddFields() {
    RenderPass::AddFields();
}

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
        if (fb_target->rend_fbo.Get()) {  // XXXX TEMPORARY!
            //auto rend_id = renderer.GetResourceGlId(fb_target->rend_fbo.Get());
            //std::cerr << "XXXX Rendering to " << rend_id << "\n";
            renderer.BindFramebuffer(fb_target->rend_fbo);
        }
        else {
            ASSERT(fb_target->target_fb >= 0);
            ASSERT(fb_target->color_fb  >  0);
            ASSERT(fb_target->depth_fb  >  0);
            gm.BindFramebuffer(GL_FRAMEBUFFER, fb_target->target_fb);
            gm.FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                    GL_TEXTURE_2D, fb_target->color_fb, 0);
            gm.FramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                    GL_TEXTURE_2D, fb_target->depth_fb, 0);
        }
    }
    else {
        renderer.BindFramebuffer(ion::gfx::FramebufferObjectPtr());
    }

    // This is required in VR to keep the controller geometry from being
    // clipped by the near plane.
    gm.Enable(GL_DEPTH_CLAMP);

    renderer.DrawScene(ion_root);

    if (fb_target && fb_target->rend_fbo.Get()) {  // XXXX TEMPORARY!
        const uint32 kMask =
            ion::gfx::Renderer::kColorBufferBit |
            ion::gfx::Renderer::kDepthBufferBit;
        renderer.ResolveMultisampleFramebuffer(fb_target->rend_fbo,
                                               fb_target->dest_fbo, kMask);
#if XXXX
        // XXXX ----------------
        auto dest_id = renderer.GetResourceGlId(fb_target->dest_fbo.Get());
        gm.BindFramebuffer(GL_READ_FRAMEBUFFER, dest_id);
        uint8 pdata[100 * 4];
        gm.ReadPixels(0, 0, 10, 10, GL_RGBA, GL_UNSIGNED_BYTE, &pdata);
        std::cerr << "XXXX PIXELS: [";
        for (size_t i = 0; i < 100 * 4; ++i)
            std::cerr << " " << static_cast<uint32>(pdata[i]);
        std::cerr << " ]\n";
        gm.BindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        // XXXX ----------------
#endif
    }

#if XXXX
    if (fb_target && fb_target->render_framebuffer_id >= 0) {  // XXXX TEMPORARY!
        gm.BindFramebuffer(GL_FRAMEBUFFER, 0);

        gm.Disable(GL_MULTISAMPLE);

        gm.BindFramebuffer(GL_READ_FRAMEBUFFER,
                           fb_target->render_framebuffer_id);
        gm.BindFramebuffer(GL_DRAW_FRAMEBUFFER,
                           fb_target->resolve_framebuffer_id);

        const auto vp_size = data.viewport.GetSize();
        gm.BlitFramebuffer(0, 0, vp_size[0], vp_size[1], 0, 0,
                           vp_size[0], vp_size[1],
                           GL_COLOR_BUFFER_BIT, GL_LINEAR);

        gm.BindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        gm.BindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        // XXXX ----------------
        //gm.Enable(GL_DEBUG_OUTPUT);
        //gm.DebugMessageCallback(MessageCallback, 0);
        gm.BindFramebuffer(GL_READ_FRAMEBUFFER,
                           fb_target->resolve_framebuffer_id);
        uint8 pdata[100 * 4];
        gm.ReadPixels(0, 0, 10, 10, GL_RGBA, GL_UNSIGNED_BYTE, &pdata);
        std::cerr << "XXXX PIXELS: [";
        for (size_t i = 0; i < 100 * 4; ++i)
            std::cerr << " " << static_cast<uint32>(pdata[i]);
        std::cerr << " ]\n";
        gm.BindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        // XXXX ----------------

        gm.Enable(GL_MULTISAMPLE);
    }
#endif
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
