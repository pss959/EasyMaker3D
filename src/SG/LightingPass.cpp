#include "SG/LightingPass.h"

#include <assert.h>

#include <ion/gfx/graphicsmanager.h>
#include <ion/gfx/node.h>
#include <ion/gfx/shaderinputregistry.h>
#include <ion/gfx/statetable.h>

#include "SG/SpecBuilder.h"
#include "SG/Math.h"
#include "SG/Node.h"
#include "View.h"

namespace SG {

void LightingPass::SetUpIon(IonContext &context) {
    NodePtr root = GetRootNode();
    if (! root)
        return;

    root->SetUpIon(context);
    added_uniforms_ = false;
}

void LightingPass::Render(ion::gfx::Renderer &renderer, PassData &data) {
    const ion::gfx::NodePtr root = GetIonRoot();

    // Set the viewport in the StateTable.
    ASSERT(root->GetStateTable());
    root->GetStateTable()->SetViewport(data.viewport);

    const int light_count = static_cast<int>(data.per_light.size());

    // Create required uniforms if not already done.
    if (! added_uniforms_) {
        auto &reg = ion::gfx::ShaderInputRegistry::GetGlobalRegistry();
        Matrix4f ident = Matrix4f::Identity();
        root->AddUniform(
            reg->Create<ion::gfx::Uniform>("uProjectionMatrix", ident));
        root->AddUniform(
            reg->Create<ion::gfx::Uniform>("uModelviewMatrix", ident));
        root->AddUniform(
            reg->Create<ion::gfx::Uniform>("uViewportSize", Vector2i(0, 0)));
        added_uniforms_ = true;
    }

    // Set global uniforms.
    root->SetUniformByName("uProjectionMatrix", data.proj_matrix);
    root->SetUniformByName("uModelviewMatrix",  data.view_matrix);

    // Set per-light uniforms.
    root->SetUniformByName("uLightCount", light_count);
    for (int i = 0; i < light_count; ++i) {
        PassData::LightData &ldata = data.per_light[i];
        root->SetUniformByNameAt("uLightPos",   i, ldata.position);
        root->SetUniformByNameAt("uLightColor", i, ldata.color);
        std::cerr << "XXXX Set uLightColor " << i << " to " << ldata.color
                  << " in " << root->GetLabel() << "\n";
        int xxxx = root->GetUniformIndex("uLightColor");
        std::cerr << "XXXX Found uLightColor at index " << xxxx << "\n";
        ion::gfx::Uniform xxxxu = root->GetUniforms()[xxxx];
        Vector3f xxxxlc = static_cast<Vector3f>(xxxxu.GetValueAt<ion::math::VectorBase3f>(i));
        std::cerr << "XXXX uLightColor " << i << " = " << xxxxlc
                  << " in " << root->GetLabel() << "\n";
        root->SetUniformByNameAt("uBiasMatrix", i, ldata.bias_matrix);
        root->SetUniformByNameAt("uDepthRange", i, ldata.depth_range);
        root->SetUniformByNameAt("uShadowMap",  i, ldata.shadow_map);
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
    renderer.DrawScene(root);
}

Parser::ObjectSpec LightingPass::GetObjectSpec() {
    // This does not add any fields.
    return Parser::ObjectSpec{
        "LightingPass", false, []{ return new LightingPass; },
        RenderPass::GetObjectSpec().field_specs };
}

}  // namespace SG
