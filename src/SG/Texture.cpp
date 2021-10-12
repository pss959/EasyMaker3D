#include "SG/Texture.h"

namespace SG {

void Texture::AddFields() {
    AddField(count_);
    AddField(uniform_name_);
    AddField(image_);
    AddField(sampler_);
}

ion::gfx::TexturePtr Texture::SetUpIon(const IonContextPtr &ion_context) {
    if (! ion_texture_) {
        if (GetName() == "ShadowMapTex") // XXXX
            std::cerr << "XXXX Setting up Ion for " << GetDesc() << "\n";

        ion_texture_.Reset(new ion::gfx::Texture);
        ion_texture_->SetLabel(GetName());

        if (auto &image = GetImage())
            ion_texture_->SetImage(0U, image->SetUpIon(ion_context));
        if (auto &sampler = GetSampler())
            ion_texture_->SetSampler(sampler->SetUpIon());

        if (! GetSampler())
            std::cerr << "XXXX " << GetDesc() << " has no SG::Sampler\n";
        if (! ion_texture_->GetSampler())
            std::cerr << "XXXX " << GetDesc() << " has no Ion sampler\n";

    }
    return ion_texture_;
}

}  // namespace SG
