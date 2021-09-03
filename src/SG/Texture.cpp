#include "SG/Texture.h"

namespace SG {

void Texture::AddFields() {
    AddField(count_);
    AddField(uniform_name_);
    AddField(image_);
    AddField(sampler_);
}

void Texture::SetUpIon(IonContext &context) {
    if (! ion_texture_) {
        ion_texture_.Reset(new ion::gfx::Texture);
        ion_texture_->SetLabel(GetName());
        if (auto &image = GetImage()) {
            image->SetUpIon(context);
            ion_texture_->SetImage(0U, image->GetIonImage());
        }
        if (auto &sampler = GetSampler()) {
            sampler->SetUpIon(context);
            ion_texture_->SetSampler(sampler->GetIonSampler());
        }
    }
}

}  // namespace SG
