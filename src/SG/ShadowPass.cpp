#include "SG/ShadowPass.h"

#include <assert.h>

#include <ion/gfx/framebufferobject.h>
#include <ion/gfx/image.h>
#include <ion/gfx/sampler.h>
#include <ion/gfx/shaderinputregistry.h>
#include <ion/gfx/texture.h>
#include <ion/math/transformutils.h>

#include "Math/Types.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "Util/General.h"
#include "Util/Tuning.h"

using ion::gfx::FramebufferObject;

namespace SG {

void ShadowPass::Render(ion::gfx::Renderer &renderer, RenderData &data,
                        const FBTarget *fb_target) {
    // ShadowPass ignores the FBTarget because it always renders to a texture.

    // Find each Node that uses a Shadow-related shader.
    const std::vector<SG::NodePtr> nodes =
        FindNodesMatchingShaderName(data.root_node, "Shadow");
    ASSERT(! nodes.empty());
    for (auto &node: nodes)
        SetShaderUniforms_(data, *node);

    // Set the viewport in the root StateTable to the texture size.
    ASSERT(data.root_node);
    const ion::gfx::NodePtr ion_root = data.root_node->GetIonNode();
    ASSERT(ion_root);
    ASSERT(ion_root->GetStateTable());
    const Vector2i viewport_size(TK::kDepthMapSize, TK::kDepthMapSize);
    ion_root->GetStateTable()->SetViewport(
        Range2i::BuildWithSize(Point2i(0, 0), viewport_size));

    auto &block = data.root_node->GetUniformBlockForPass(GetName());
    ASSERT(block.GetIonUniformBlock());
    auto &ion_block = *block.GetIonUniformBlock();

    // Render shadows for each light.
    for (size_t i = 0; i < data.per_light.size(); ++i) {
        RenderData::LightData &ldata = data.per_light[i];
        SetPerLightData_(per_light_[i], ldata);

        // Set uniforms that change for each light pass.
        ion_block.SetUniformByName("uLightMatrix", ldata.light_matrix);
        ion_block.SetUniformByName("uCastShadows",
                                   are_shadows_enabled_ && ldata.casts_shadows);

        renderer.BindFramebuffer(per_light_[i].fbo);
        renderer.DrawScene(data.root_node->GetIonNode());
    }
}

void ShadowPass::SetShaderUniforms_(RenderData &data, Node &node) {
    // If the number of lights is larger, update the per-light data.
    if (data.per_light.size() > per_light_.size()) {
        per_light_.resize(data.per_light.size());
        for (size_t i = 0; i < data.per_light.size(); ++i)
            CreatePerLightData_(data, i);
    }

    auto &block = node.GetUniformBlockForPass(GetName());
    ASSERT(block.GetIonUniformBlock());
    auto &ion_block = *block.GetIonUniformBlock();

    // The ShadowPass has global uniforms that are not used for shadows but are
    // required for some nodes; the values here do not matter. Create them if
    // not already done. Use the global registry so that all Ion shaders (such
    // as for TextNodes) can use them.
    if (! were_uniforms_created_) {
        auto &reg = *ion::gfx::ShaderInputRegistry::GetGlobalRegistry();
        const Matrix4f ident = Matrix4f::Identity();
        auto mat_func = [&ion_block, &reg, &ident](const std::string &name){
            ion_block.AddUniform(reg.Create<ion::gfx::Uniform>(name, ident));
            ion_block.SetUniformByName(name, ident);
        };
        mat_func("uProjectionMatrix");
        mat_func("uModelviewMatrix");
        mat_func("uModelMatrix");
        mat_func("uViewMatrix");
        const Vector2i viewport_size(TK::kDepthMapSize, TK::kDepthMapSize);
        ion_block.AddUniform(reg.Create<ion::gfx::Uniform>("uViewportSize",
                                                           viewport_size));
        were_uniforms_created_ = true;
    }
}

void ShadowPass::CreatePerLightData_(RenderData &data, size_t index) {
    // Create an Image in which to store depth values.
    ion::gfx::ImagePtr image(new ion::gfx::Image);
    image->Set(ion::gfx::Image::kRenderbufferDepth24,
               TK::kDepthMapSize, TK::kDepthMapSize,
               ion::base::DataContainerPtr());

    // Create a Sampler for the texture.
    ion::gfx::SamplerPtr sampler(new ion::gfx::Sampler);
    sampler->SetMinFilter(ion::gfx::Sampler::kLinear);
    sampler->SetMagFilter(ion::gfx::Sampler::kLinear);
    sampler->SetWrapS(ion::gfx::Sampler::kClampToEdge);
    sampler->SetWrapT(ion::gfx::Sampler::kClampToEdge);

    // Create the Depth texture.
    ion::gfx::TexturePtr tex(new ion::gfx::Texture);
    tex->SetLabel("Shadow Depth Map Texture " + Util::ToString(index));
    tex->SetSampler(sampler);
    tex->SetImage(0U, image);

    // FBO holding the texture.
    ion::gfx::FramebufferObjectPtr fbo(
        new FramebufferObject(TK::kDepthMapSize, TK::kDepthMapSize));
    fbo->SetLabel("Shadow Depth FBO");
    fbo->SetColorAttachment(0U, ion::gfx::FramebufferObject::Attachment());
    fbo->SetDepthAttachment(FramebufferObject::Attachment(tex));

    per_light_[index].texture = tex;
    per_light_[index].fbo     = fbo;
}

void ShadowPass::SetPerLightData_(const PerLight_ &pldata,
                                  RenderData::LightData &ldata) {
    ldata.shadow_map = pldata.texture;

    // Use orthographic projection to be able to have a negative near distance
    // so objects behind the lights have reasonable depths.
    const float hs = TK::kShadowPassLightHalfSize;
    const float nr = TK::kShadowPassLightNear;
    const float fr = TK::kShadowPassLightFar;
    ldata.light_matrix =
        ion::math::OrthographicMatrixFromFrustum(-hs, hs, -hs, hs, nr, fr) *
        ion::math::LookAtMatrixFromCenter(ldata.position, Point3f::Zero(),
                                          Vector3f::AxisY());
}

}  // namespace SG
