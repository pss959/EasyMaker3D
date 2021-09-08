#include "SG/LightingPass.h"

#include <assert.h>

#include <ion/gfx/graphicsmanager.h>
#include <ion/gfx/node.h>
#include <ion/gfx/shaderinputregistry.h>
#include <ion/gfx/statetable.h>

#include "SG/Node.h"
#include "SG/UniformBlock.h"
#include "View.h"

namespace SG {

void LightingPass::AddFields() {
    RenderPass::AddFields();
}

void LightingPass::SetUpIon(const ContextPtr &context) {
    RenderPass::SetUpIon(context);

    NodePtr root = GetRootNode();
    if (! root)
        return;

    root->SetUpIon(context);
}

void LightingPass::Render(ion::gfx::Renderer &renderer, PassData &data) {
    const NodePtr           root = GetRootNode();
    const ion::gfx::NodePtr ion_root = GetIonRoot();

    // Set the viewport in the StateTable.
    ASSERT(ion_root->GetStateTable());
    ion_root->GetStateTable()->SetViewport(data.viewport);

    const int light_count = static_cast<int>(data.per_light.size());

    UniformBlockPtr block =
        root->GetUniformBlockForPass("Lighting Pass", false);
    ASSERT(block && block->GetIonUniformBlock());
    ion::gfx::UniformBlock &ion_block = *block->GetIonUniformBlock();

    // Set global uniforms.
    ion_block.SetUniformByName("uViewportSize",     data.viewport.GetSize());
    ion_block.SetUniformByName("uProjectionMatrix", data.proj_matrix);
    ion_block.SetUniformByName("uViewMatrix",       data.view_matrix);
    ion_block.SetUniformByName("uModelMatrix",      Matrix4f::Identity());
    ion_block.SetUniformByName("uModelviewMatrix",  data.view_matrix);
    ion_block.SetUniformByName("uViewPos",          data.view_pos);
    ion_block.SetUniformByName("uLightCount",       light_count);

    // Set per-light uniforms.
    for (int i = 0; i < light_count; ++i) {
        PassData::LightData &ldata = data.per_light[i];
        ion_block.SetUniformByNameAt("uLightPos",       i, ldata.position);
        ion_block.SetUniformByNameAt("uLightColor",     i, ldata.color);
        ion_block.SetUniformByNameAt("uLightMatrix",    i, ldata.light_matrix);
        ion_block.SetUniformByNameAt("uLightShadowMap", i, ldata.shadow_map);
    }

    // Set up the framebuffer(s).
    ion::gfx::GraphicsManager &gm = *renderer.GetGraphicsManager();
    if (data.fb_target) {
        const auto &fbt = *data.fb_target;
        ASSERT(fbt.target_fb >= 0);
        ASSERT(fbt.color_fb  >  0);
        ASSERT(fbt.depth_fb  >  0);
        gm.BindFramebuffer(GL_FRAMEBUFFER, fbt.target_fb);
        gm.FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                GL_TEXTURE_2D, fbt.color_fb, 0);
        gm.FramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                GL_TEXTURE_2D, fbt.depth_fb, 0);
    }
    else {
        renderer.BindFramebuffer(ion::gfx::FramebufferObjectPtr());
    }
    renderer.DrawScene(ion_root);
}

}  // namespace SG
