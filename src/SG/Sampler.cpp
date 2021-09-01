#include "SG/Sampler.h"

#include "SG/SpecBuilder.h"

namespace SG {

void Sampler::SetUpIon(IonContext &context) {
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

Parser::ObjectSpec Sampler::GetObjectSpec() {
    SG::SpecBuilder<Sampler> builder;
    builder.AddBool("auto_mipmaps",              &Sampler::auto_mipmaps_);
    builder.AddEnum<CompareMode>("compare_mode", &Sampler::compare_mode_);
    builder.AddEnum<CompareFunction>(
        "compare_function",                      &Sampler::compare_function_);
    builder.AddEnum<FilterMode>("min_filter",    &Sampler::min_filter_);
    builder.AddEnum<FilterMode>("mag_filter",    &Sampler::mag_filter_);
    builder.AddEnum<WrapMode>("wrap_r_mode",     &Sampler::wrap_r_mode_);
    builder.AddEnum<WrapMode>("wrap_s_mode",     &Sampler::wrap_s_mode_);
    builder.AddEnum<WrapMode>("wrap_t_mode",     &Sampler::wrap_t_mode_);
    builder.AddFloat("max_anisotropy",           &Sampler::max_anisotropy_);
    builder.AddFloat("min_lod",                  &Sampler::min_lod_);
    builder.AddFloat("max_lod",                  &Sampler::max_lod_);
    return Parser::ObjectSpec{
        "Sampler", false, []{ return new Sampler; }, builder.GetSpecs() };
}

}  // namespace SG
