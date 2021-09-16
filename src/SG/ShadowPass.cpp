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

using ion::gfx::FramebufferObject;

namespace SG {

//! Size used for depth map.
static const int kDepthMapSize = 2048;

void ShadowPass::AddFields() {
    RenderPass::AddFields();
}

void ShadowPass::SetUniforms(PassData &data) {
    // If the number of lights is larger, update the per-light data.
    if (data.per_light.size() > per_light_.size()) {
        per_light_.resize(data.per_light.size());
        for (size_t i = 0; i < data.per_light.size(); ++i)
            CreatePerLightData_(data, i);
    }

    const NodePtr           root = GetRootNode();
    const ion::gfx::NodePtr ion_root = root->GetIonNode();

    // Make sure the viewport is the same size as the texture.
    ASSERT(ion_root->GetStateTable());
    const Vector2i viewport_size(kDepthMapSize, kDepthMapSize);
    ion_root->GetStateTable()->SetViewport(
        Range2i::BuildWithSize(Point2i(0, 0), viewport_size));


    ShaderProgramPtr program = GetDefaultShaderProgram();
    ASSERT(program && program->GetUniformBlock() &&
           program->GetUniformBlock()->GetIonUniformBlock());
    auto &ion_block = *program->GetUniformBlock()->GetIonUniformBlock();

    // Set uniforms that do not change but are required for some nodes.
    const Matrix4f ident = Matrix4f::Identity();
    ion_block.SetUniformByName("uProjectionMatrix", ident);
    ion_block.SetUniformByName("uModelviewMatrix",  ident);
    ion_block.SetUniformByName("uModelMatrix",      ident);
    ion_block.SetUniformByName("uViewMatrix",       ident);
}

void ShadowPass::Render(ion::gfx::Renderer &renderer, PassData &data) {
    ShaderProgramPtr program = GetDefaultShaderProgram();
    auto &ion_block = *program->GetUniformBlock()->GetIonUniformBlock();

    // Render shadows for each light.
    for (size_t i = 0; i < data.per_light.size(); ++i) {
        PassData::LightData &ldata = data.per_light[i];
        SetPerLightData_(per_light_[i], ldata);

        // Set uniforms that change for each light pass.
        ion_block.SetUniformByName("uLightMatrix", ldata.light_matrix);
        ion_block.SetUniformByName("uCastShadows", ldata.casts_shadows);

        renderer.BindFramebuffer(per_light_[i].fbo);
        renderer.DrawScene(GetRootNode()->GetIonNode());
    }
}

void ShadowPass::CreatePerLightData_(PassData &data, size_t index) {
    // Create an Image in which to store depth values.
    ion::gfx::ImagePtr image(new ion::gfx::Image);
    image->Set(ion::gfx::Image::kRenderbufferDepth24,
               kDepthMapSize, kDepthMapSize,
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
        new FramebufferObject(kDepthMapSize, kDepthMapSize));
    fbo->SetLabel("Shadow Depth FBO");
    fbo->SetColorAttachment(0U, ion::gfx::FramebufferObject::Attachment());
    fbo->SetDepthAttachment(FramebufferObject::Attachment(tex));

    per_light_[index].texture = tex;
    per_light_[index].fbo     = fbo;
}

void ShadowPass::SetPerLightData_(const PerLight_ &pldata,
                                  PassData::LightData &ldata) {
    ldata.shadow_map = pldata.texture;

    // Use orthographic projection to be able to have a negative near distance
    // so objects behind the lights have reasonable depths.
    // XXXX Get real values from somewhere?
    const float s    = 80.f;
    const float near = -20.f;
    const float far  = 202.f;
    ldata.light_matrix =
        ion::math::OrthographicMatrixFromFrustum(-s, s, -s, s, near, far) *
        ion::math::LookAtMatrixFromCenter(ldata.position, Point3f::Zero(),
                                          Vector3f::AxisY());
}

}  // namespace SG
