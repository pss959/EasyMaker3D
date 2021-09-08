#include "SG/Sampler.h"

namespace SG {

void Sampler::AddFields() {
    AddField(auto_mipmaps_);
    AddField(compare_mode_);
    AddField(compare_function_);
    AddField(min_filter_);
    AddField(mag_filter_);
    AddField(wrap_r_mode_);
    AddField(wrap_s_mode_);
    AddField(wrap_t_mode_);
    AddField(max_anisotropy_);
    AddField(min_lod_);
    AddField(max_lod_);
}

void Sampler::SetUpIon(const ContextPtr &context) {
    Object::SetUpIon(context);

    if (! ion_sampler_) {
        ion_sampler_.Reset(new ion::gfx::Sampler);
        ion_sampler_->SetAutogenerateMipmapsEnabled(auto_mipmaps_);
        ion_sampler_->SetCompareMode(compare_mode_);
        ion_sampler_->SetCompareFunction(compare_function_);
        ion_sampler_->SetMinFilter(min_filter_);
        ion_sampler_->SetMagFilter(mag_filter_);
        ion_sampler_->SetWrapR(wrap_r_mode_);
        ion_sampler_->SetWrapS(wrap_s_mode_);
        ion_sampler_->SetWrapT(wrap_t_mode_);
        ion_sampler_->SetMaxAnisotropy(max_anisotropy_);
        ion_sampler_->SetMinLod(min_lod_);
        ion_sampler_->SetMaxLod(max_lod_);
    }
}

}  // namespace SG
