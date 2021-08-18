#include "SG/Texture.h"

#include "SG/Image.h"
#include "SG/Sampler.h"
#include "SG/SpecBuilder.h"

namespace SG {

void Texture::Finalize() {
    assert(! ion_texture_);
    ion_texture_.Reset(new ion::gfx::Texture);
    if (image_)
        ion_texture_->SetImage(0U, image_->GetIonImage());
    if (sampler_)
        ion_texture_->SetSampler(sampler_->GetIonSampler());
}

NParser::ObjectSpec Texture::GetObjectSpec() {
    SG::SpecBuilder<Texture> builder;
    builder.AddString("uniform_name",     &Texture::uniform_name_);
    builder.AddObject<Image>("image",     &Texture::image_);
    builder.AddObject<Sampler>("sampler", &Texture::sampler_);
    return NParser::ObjectSpec{
        "Texture", false, []{ return new Texture; }, builder.GetSpecs() };
}

}  // namespace SG
