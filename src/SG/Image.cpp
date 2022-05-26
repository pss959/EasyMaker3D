#include "SG/Image.h"

#include "SG/IonContext.h"
#include "SG/Uniform.h"
#include "Util/Assert.h"

namespace SG {

void Image::AddFields() {
    AddField(sub_images_);
    Object::AddFields();
}

void Image::CreationDone() {
    for (const auto &sub: sub_images_.GetValue()) {
        ASSERT(! sub->GetName().empty());
        sub_image_map_[sub->GetName()] = sub;
    }
}

ion::gfx::ImagePtr Image::SetUpIon(const IonContextPtr &ion_context) {
    if (! ion_image_)
        ion_image_ = CreateIonImage(ion_context->GetTracker());
    return ion_image_;
}

SubImagePtr Image::FindSubImage(const std::string &name) const {
    SubImagePtr sub;
    const auto it = sub_image_map_.find(name);
    if (it != sub_image_map_.end())
        sub = it->second;
    return sub;
}

void Image::ReplaceImage(const ion::gfx::ImagePtr new_image) {
    ASSERT(ion_image_);
    ion_image_ = new_image;
    ProcessChange(Change::kAppearance, *this);
}

}  // namespace SG
