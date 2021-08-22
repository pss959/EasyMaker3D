#include "SG/Sampler.h"

#include "SG/SpecBuilder.h"

namespace SG {

void Sampler::SetUpIon(IonContext &context) {
    if (! ion_sampler_) {
        ion_sampler_.Reset(new ion::gfx::Sampler);
        ion_sampler_->SetWrapS(wrap_s_mode_);
        ion_sampler_->SetWrapT(wrap_t_mode_);
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
