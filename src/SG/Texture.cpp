#include "SG/Texture.h"

#include "SG/IonContext.h"

namespace SG {

Texture::~Texture() {
    if (IsCreationDone() && ! IsTemplate() && GetImage())
        Unobserve(*GetImage());
}

void Texture::AddFields() {
    AddField(count_.Init("count", 1));
    AddField(uniform_name_.Init("uniform_name"));
    AddField(image_.Init("image"));
    AddField(sampler_.Init("sampler"));

    Object::AddFields();
}

void Texture::CreationDone() {
    Object::CreationDone();

    // Detect changes to the Image.
    if (! IsTemplate() && GetImage())
        Observe(*GetImage());
}

bool Texture::ProcessChange(Change change, const Object &obj) {
    if (! Object::ProcessChange(change, obj))
        return false;

    if (ion_texture_ && &obj == GetImage().get())
        ion_texture_->SetImage(0U, GetImage()->GetIonImage());
    return true;
}

ion::gfx::TexturePtr Texture::SetUpIon(const IonContextPtr &ion_context) {
    if (! ion_texture_) {
        ion_texture_.Reset(new ion::gfx::Texture);
        ion_texture_->SetLabel(GetName());

        if (auto &image = GetImage())
            ion_texture_->SetImage(0U, image->SetUpIon(ion_context));
        if (auto &sampler = GetSampler())
            ion_texture_->SetSampler(sampler->SetUpIon());
    }
    return ion_texture_;
}

}  // namespace SG
