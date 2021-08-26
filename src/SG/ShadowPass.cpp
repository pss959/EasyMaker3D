#include "SG/ShadowPass.h"

#include <assert.h>

#include <ion/gfx/framebufferobject.h>
#include <ion/gfx/image.h>
#include <ion/gfx/sampler.h>
#include <ion/gfx/shaderinputregistry.h>
#include <ion/gfx/texture.h>
#include <ion/math/transformutils.h>

#include "SG/SpecBuilder.h"
#include "SG/Math.h"
#include "SG/Node.h"

using ion::gfx::FramebufferObject;

namespace SG {

//! Size used for depth map.
static const int kDepthMapSize = 512; // XXXX Make it 2048;

void ShadowPass::SetUpIon(IonContext &context) {
    NodePtr root = GetRootNode();
    if (! root)
        return;

    root->SetUpIon(context);
    InitGlobalUniforms();

    // Make sure the viewport is the same size as the texture.
    const ion::gfx::NodePtr ion_root = GetIonRoot();
    ASSERT(ion_root->GetStateTable());
    const Vector2i viewport_size(kDepthMapSize, kDepthMapSize);
    ion_root->GetStateTable()->SetViewport(
        Range2i::BuildWithSize(Point2i(0, 0), viewport_size));
}

void ShadowPass::Render(ion::gfx::Renderer &renderer, PassData &data) {
    // If the number of lights is larger, update the per-light data.
    if (data.per_light.size() > per_light_.size()) {
        per_light_.resize(data.per_light.size());
        for (size_t i = 0; i < data.per_light.size(); ++i)
            CreatePerLightData_(data, i);
    }

    const ion::gfx::NodePtr root = GetIonRoot();

    // Render shadows for each light.
    for (size_t i = 0; i < data.per_light.size(); ++i) {
        PassData::LightData &ldata = data.per_light[i];
        SetPerLightData_(ldata);

        root->SetUniformByName("uModelviewMatrix", ldata.shadow_matrix);
        root->SetUniformByName("uLightPos",     ldata.position);
        root->SetUniformByName("uShadowMatrix", ldata.shadow_matrix);
        root->SetUniformByName("uDepthRange",   ldata.depth_range);

        renderer.BindFramebuffer(per_light_[i].fbo);
        renderer.DrawScene(root);
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

    // FBO.
    ion::gfx::FramebufferObjectPtr fbo(
        new FramebufferObject(kDepthMapSize, kDepthMapSize));
    fbo->SetLabel("Shadow Depth FBO");
    fbo->SetColorAttachment(0U, ion::gfx::FramebufferObject::Attachment());
    fbo->SetDepthAttachment(FramebufferObject::Attachment(tex));

    per_light_[index].texture = tex;
    per_light_[index].fbo     = fbo;
}

void ShadowPass::SetPerLightData_(PassData::LightData &data) {
    // Compute the matrices and depth range from the light position and scene
    // radius.
    // XXXX Use a real radius value from somewhere?
    const float radius = 100.f;
    const float light_dist = ion::math::Length(data.position - Point3f::Zero());
    const float min_depth = light_dist - radius;
    const float max_depth = light_dist + radius;

    const Anglef fov = Anglef::FromRadians(2.f * atan2f(radius, light_dist));
    const Matrix4f proj_view_mat =
        ion::math::PerspectiveMatrixFromView(fov, 1.f, min_depth, max_depth) *
        ion::math::LookAtMatrixFromCenter(data.position, Point3f::Zero(),
                                          Vector3f::AxisY());

    // The bias matrix adds a scale by 1/2 and translation by 1/2 so that
    // values in the range (-1,1) end up in the range (0,1).
    const Matrix4f bias_mat =
        ion::math::TranslationMatrix(Vector3f(.5f, .5f, .5f)) *
        (ion::math::ScaleMatrixH(Vector3f(.5f, .5f, .5f)) * proj_view_mat);

    data.depth_range   = Vector2f(min_depth, max_depth);
    data.shadow_matrix = view_mat;
    data.bias_matrix   = bias_mat;
}

Parser::ObjectSpec ShadowPass::GetObjectSpec() {
    // This does not add any fields.
    return Parser::ObjectSpec{
        "ShadowPass", false, []{ return new ShadowPass; },
        RenderPass::GetObjectSpec().field_specs };
}

}  // namespace SG
