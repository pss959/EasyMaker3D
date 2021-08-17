#include "SG/Sampler.h"

#include "SG/SpecBuilder.h"

namespace SG {

void Sampler::Finalize() {
    assert(! ion_sampler_);
    ion_sampler_.Reset(new ion::gfx::Sampler);
    ion_sampler_->SetWrapS(wrap_s_mode_);
    ion_sampler_->SetWrapT(wrap_t_mode_);
}

std::vector<NParser::FieldSpec> Sampler::GetFieldSpecs_() {
    SG::SpecBuilder<Sampler> builder;
    builder.AddEnum<WrapMode>("wrap_s_mode", &Sampler::wrap_s_mode_);
    builder.AddEnum<WrapMode>("wrap_t_mode", &Sampler::wrap_t_mode_);
    return builder.GetSpecs();
}

}  // namespace SG
