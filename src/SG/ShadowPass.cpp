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

    // Make sure the viewport is the same size as the texture.
    const ion::gfx::NodePtr ion_root = GetIonRoot();
    ASSERT(ion_root->GetStateTable());
    const Vector2i viewport_size(kDepthMapSize, kDepthMapSize);
    ion_root->GetStateTable()->SetViewport(
        Range2i::BuildWithSize(Point2i(0, 0), viewport_size));

    // Set uniforms that do not change but are required for some nodes.
    const Matrix4f ident = Matrix4f::Identity();
    ion_root->SetUniformByName("uProjectionMatrix", ident);
    ion_root->SetUniformByName("uModelviewMatrix",  ident);
    ion_root->SetUniformByName("uModelMatrix",      ident);
    ion_root->SetUniformByName("uViewMatrix",       ident);
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
        SetPerLightData_(per_light_[i], ldata);

        // Set uniforms
        root->SetUniformByName("uLightMatrix", ldata.light_matrix);

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

#if 0 // XXXX
    // Use a reasonable field of view and depth range.
    const Anglef fov = Anglef::FromDegrees(66.f);
    const float min_depth = 1.f;
    const float max_depth = 100.f;

    // Compute the matrix with the projection and view relative to the light.
    ldata.light_matrix =
        ion::math::PerspectiveMatrixFromView(fov, 1.f, min_depth, max_depth) *
        ion::math::LookAtMatrixFromCenter(ldata.position, Point3f::Zero(),
                                          Vector3f::AxisY());
#endif

#if 1 // XXXX
    // Use orthographic projection to be able to have a negative near distance
    // so objects behind the lights have reasonable depths.
    const float s    = 80.f;
    const float near = -20.f;
    const float far  = 202.f;
    ldata.light_matrix =
        ion::math::OrthographicMatrixFromFrustum(-s, s, -s, s, near, far) *
        ion::math::LookAtMatrixFromCenter(ldata.position, Point3f::Zero(),
                                          Vector3f::AxisY());
#endif
}

Parser::ObjectSpec ShadowPass::GetObjectSpec() {
    // This does not add any fields.
    return Parser::ObjectSpec{
        "ShadowPass", false, []{ return new ShadowPass; },
        RenderPass::GetObjectSpec().field_specs };
}

}  // namespace SG
