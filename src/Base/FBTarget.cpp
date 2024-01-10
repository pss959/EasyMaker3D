#include "Base/FBTarget.h"

#include <ion/gfx/image.h>
#include <ion/gfx/sampler.h>
#include <ion/gfx/texture.h>

void FBTarget::Init(const Str &prefix, const Vector2ui &size, uint samples) {
    using ion::gfx::FramebufferObject;
    using ion::gfx::Image;
    using ion::gfx::ImagePtr;
    using ion::gfx::Sampler;
    using ion::gfx::SamplerPtr;
    using ion::gfx::Texture;
    using ion::gfx::TexturePtr;

    // Rendered FBO with multisampled color and depth/stencil attachments.
    rendered_fbo_.Reset(new FramebufferObject(size[0], size[1]));
    rendered_fbo_->SetLabel(prefix + " Rendered FBO");
    rendered_fbo_->SetColorAttachment(
        0U, FramebufferObject::Attachment::CreateMultisampled(
            Image::kRgba8888, samples));
    auto depth_stencil = FramebufferObject::Attachment::CreateMultisampled(
        Image::kRenderbufferDepth24Stencil8, samples);
    rendered_fbo_->SetDepthAttachment(depth_stencil);
    rendered_fbo_->SetStencilAttachment(depth_stencil);

    // Resolved FBO sampler, image, and texture.
    SamplerPtr sampler(new Sampler);
    sampler->SetMinFilter(Sampler::kLinear);
    sampler->SetMagFilter(Sampler::kLinear);
    sampler->SetWrapS(Sampler::kClampToEdge);
    sampler->SetWrapT(Sampler::kClampToEdge);
    ImagePtr resolved_image(new Image);
    resolved_image->Set(Image::kRgba8888, size[0], size[1],
                        ion::base::DataContainerPtr());
    TexturePtr resolved_tex(new Texture);
    resolved_tex->SetLabel(prefix + " Resolved Texture");
    resolved_tex->SetSampler(sampler);
    resolved_tex->SetMaxLevel(0);
    resolved_tex->SetImage(0U, resolved_image);

    // Resolved FBO.
    resolved_fbo_.Reset(new FramebufferObject(size[0], size[1]));
    resolved_fbo_->SetLabel(prefix + " Resolved FBO");
    resolved_fbo_->SetColorAttachment(
        0U, FramebufferObject::Attachment(resolved_tex));
}
