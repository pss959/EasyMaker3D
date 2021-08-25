#include "SG/Texture.h"

#include "SG/Image.h"
#include "SG/Sampler.h"
#include "SG/SpecBuilder.h"

namespace SG {

void Texture::SetUpIon(IonContext &context) {
    if (! ion_texture_) {
        ion_texture_.Reset(new ion::gfx::Texture);
        ion_texture_->SetLabel(GetName());
        if (image_) {
            image_->SetUpIon(context);
            ion_texture_->SetImage(0U, image_->GetIonImage());
        }
        if (sampler_) {
            sampler_->SetUpIon(context);
            ion_texture_->SetSampler(sampler_->GetIonSampler());
        }
    }
}

Parser::ObjectSpec Texture::GetObjectSpec() {
    SG::SpecBuilder<Texture> builder;
    builder.AddInt("count",               &Texture::count_);
    builder.AddString("uniform_name",     &Texture::uniform_name_);
    builder.AddObject<Image>("image",     &Texture::image_);
    builder.AddObject<Sampler>("sampler", &Texture::sampler_);
    return Parser::ObjectSpec{
        "Texture", false, []{ return new Texture; }, builder.GetSpecs() };
}

}  // namespace SG
