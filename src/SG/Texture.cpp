#include "SG/Texture.h"

namespace SG {

void Texture::AddFields() {
    AddField(count_);
    AddField(uniform_name_);
    AddField(image_);
    AddField(sampler_);
}

void Texture::CreateIonTexture() {
    ASSERT(! ion_texture_);
    ion_texture_.Reset(new ion::gfx::Texture);
    ion_texture_->SetLabel(GetName());
}

}  // namespace SG
