#include "SG/LightingPass.h"

#include <assert.h>

#include <ion/gfx/graphicsmanager.h>
#include <ion/gfx/node.h>
#include <ion/gfx/shaderinputregistry.h>
#include <ion/gfx/statetable.h>

#include "FBTarget.h"
#include "SG/Node.h"
#include "SG/UniformBlock.h"

namespace SG {

void LightingPass::AddFields() {
    RenderPass::AddFields();
}

void LightingPass::SetUniforms(PassData &data) {
    ShaderProgramPtr program = GetDefaultShaderProgram();
    ASSERT(program);
    auto &block = program->GetUniformBlock();
    ASSERT(block && block->GetIonUniformBlock());
    auto &ion_block = *block->GetIonUniformBlock();

    // Set pass-independent uniforms.
    block->SetModelMatrices(Matrix4f::Identity(), data.view_matrix);

    // Set global pass-specific uniforms.
    const int light_count = static_cast<int>(data.per_light.size());
    SetIonUniform(ion_block, "uViewportSize",     data.viewport.GetSize());
    SetIonUniform(ion_block, "uProjectionMatrix", data.proj_matrix);
    SetIonUniform(ion_block, "uViewMatrix",       data.view_matrix);
    SetIonUniform(ion_block, "uViewPos",          data.view_pos);
    SetIonUniform(ion_block, "uLightCount",       light_count);

    // Set per-light uniforms.
    for (int i = 0; i < light_count; ++i) {
        PassData::LightData &ldata = data.per_light[i];
        SetIonUniformAt(ion_block, "uLightPos",       i, ldata.position);
        SetIonUniformAt(ion_block, "uLightColor",     i, ldata.color);
        SetIonUniformAt(ion_block, "uLightMatrix",    i, ldata.light_matrix);
        SetIonUniformAt(ion_block, "uLightShadowMap", i, ldata.shadow_map);
    }
}

void LightingPass::Render(ion::gfx::Renderer &renderer, PassData &data) {
    const NodePtr           root = GetRootNode();
    const ion::gfx::NodePtr ion_root = root->GetIonNode();

    // Set the viewport in the StateTable.
    ASSERT(ion_root->GetStateTable());
    ion_root->GetStateTable()->SetViewport(data.viewport);

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

    // This is required in VR to keep the controller geometry from being
    // clipped by the near plane.
    gm.Enable(GL_DEPTH_CLAMP);

    renderer.DrawScene(ion_root);
}

}  // namespace SG
