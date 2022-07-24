#include "SG/Sampler.h"

namespace SG {

void Sampler::AddFields() {
    AddField(auto_mipmaps_.Init("auto_mipmaps",     false));
    AddField(compare_mode_.Init("compare_mode",     CompareMode::kNone));
    AddField(compare_function_.Init("compare_function",
                                    CompareFunction::kLess));
    AddField(min_filter_.Init("min_filter",         FilterMode::kNearest));
    AddField(mag_filter_.Init("mag_filter",         FilterMode::kNearest));
    AddField(wrap_r_mode_.Init("wrap_r_mode",       WrapMode::kRepeat));
    AddField(wrap_s_mode_.Init("wrap_s_mode",       WrapMode::kRepeat));
    AddField(wrap_t_mode_.Init("wrap_t_mode",       WrapMode::kRepeat));
    AddField(max_anisotropy_.Init("max_anisotropy", 1));
    AddField(min_lod_.Init("min_lod", -1000));
    AddField(max_lod_.Init("max_lod", 1000));

    Object::AddFields();
}

ion::gfx::SamplerPtr Sampler::SetUpIon() {
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
    return ion_sampler_;
}

}  // namespace SG
