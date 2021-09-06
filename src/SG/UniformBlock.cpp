#include "SG/UniformBlock.h"

#include "Math/Types.h"

namespace SG {

void UniformBlock::AddFields() {
    AddField(material_);
    AddField(textures_);
    AddField(uniforms_);
}

void UniformBlock::SetUpIon(IonContext &context) {
    if (! ion_uniform_block_) {
        ion_uniform_block_.Reset(new ion::gfx::UniformBlock);
        ion_uniform_block_->SetLabel(GetName());

        if (GetMaterial()) {
            AddMaterialUniforms_(context, *GetMaterial());
        }
        for (const auto &tex: GetTextures()) {
            tex->SetUpIon(context);
            AddTextureUniform_(context, *tex);
        }
        for (const auto &uni: GetUniforms()) {
            uni->SetUpIon(context);
            ion_uniform_block_->AddUniform(uni->GetIonUniform());
        }
    }
}

void UniformBlock::AddMaterialUniforms_(IonContext &context,
                                        const Material &mat) {
    auto &reg = context.registry_stack.top();
    ion_uniform_block_->AddUniform(
        reg->Create<ion::gfx::Uniform>("uBaseColor", mat.GetBaseColor()));
    ion_uniform_block_->AddUniform(
        reg->Create<ion::gfx::Uniform>("uSmoothness", mat.GetSmoothness()));
    ion_uniform_block_->AddUniform(
        reg->Create<ion::gfx::Uniform>("uMetalness", mat.GetMetalness()));
}

void UniformBlock::AddTextureUniform_(IonContext &context, const Texture &tex) {
    auto              &reg   = context.registry_stack.top();
    const std::string &name  = tex.GetUniformName();
    const int          count = tex.GetCount();

    ion::gfx::Uniform u;
    if (count > 1) {
        std::vector<ion::gfx::TexturePtr> texvec(count, tex.GetIonTexture());
        u = reg->CreateArrayUniform(name, texvec.data(), count,
                                    ion::base::AllocatorPtr());
    }
    else {
        u = reg->Create<ion::gfx::Uniform>(name, tex.GetIonTexture());
    }
    ion_uniform_block_->AddUniform(u);
}

}  // namespace SG
