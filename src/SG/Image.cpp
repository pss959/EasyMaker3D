//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "SG/Image.h"

#include "SG/IonContext.h"
#include "SG/Uniform.h"
#include "Util/Assert.h"
#include "Util/KLog.h"

namespace SG {

void Image::AddFields() {
    AddField(sub_images_.Init("sub_images"));

    Object::AddFields();
}

void Image::CreationDone() {
    for (const auto &sub: sub_images_.GetValue()) {
        ASSERT(! sub->GetName().empty());
        sub_image_map_[sub->GetName()] = sub;
    }
}

ion::gfx::ImagePtr Image::SetUpIon(const IonContextPtr &ion_context) {
    KLOG('Z', ion_context->GetIndent() << "SetUpIon for " << GetDesc());
    ion_context_ = ion_context;
    if (! ion_image_)
        ion_image_ = CreateIonImage();
    return ion_image_;
}

SubImagePtr Image::FindSubImage(const Str &name) const {
    SubImagePtr sub;
    const auto it = sub_image_map_.find(name);
    if (it != sub_image_map_.end())
        sub = it->second;
    return sub;
}

FileMap & Image::GetFileMap() const {
    ASSERT(ion_context_);
    return ion_context_->GetFileMap();
}

void Image::ReplaceImage(const ion::gfx::ImagePtr new_image) {
    ASSERT(ion_image_);
    ion_image_ = new_image;
    ProcessChange(Change::kAppearance, *this);
}

}  // namespace SG
