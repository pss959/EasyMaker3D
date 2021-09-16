#include "SG/UniformBlock.h"

#include "Assert.h"
#include "Math/Types.h"

using ion::gfx::ShaderInputRegistry;

namespace SG {

void UniformBlock::AddFields() {
    AddField(material_);
    AddField(textures_);
    AddField(uniforms_);
}

void UniformBlock::CreateIonUniformBlock() {
    ASSERT(! ion_uniform_block_);
    ion_uniform_block_.Reset(new ion::gfx::UniformBlock);
    ion_uniform_block_->SetLabel(GetName());
}

void UniformBlock::AddIonUniforms() {
    ASSERT(ion_uniform_block_);
    ASSERT(ion_registry_);

    if (GetMaterial())
        AddMaterialUniforms_(*GetMaterial());
    for (const auto &tex: GetTextures())
        AddTextureUniform_(*tex);
    for (const auto &uniform: GetUniforms())
        AddIonUniform_(uniform->CreateIonUniform(*ion_registry_));
}

void UniformBlock::SetModelMatrices(const Matrix4f &model_matrix,
                                    const Matrix4f &modelview_matrix) {
    ASSERT(ion_uniform_block_);

    // Create the uniforms if not already done.
    if (mm_index_ == kInvalidIndex) {
        mm_index_ = AddIonUniform_(ion_registry_->Create<ion::gfx::Uniform>(
                                       "uModelMatrix", model_matrix));
        mv_index_ = AddIonUniform_(ion_registry_->Create<ion::gfx::Uniform>(
                                       "uModelviewMatrix", modelview_matrix));
    }
    else {
        ion_uniform_block_->SetUniformValue(mm_index_, model_matrix);
        ion_uniform_block_->SetUniformValue(mv_index_, modelview_matrix);
    }
}

void UniformBlock::SetBaseColor(const Color &color) {
    ASSERT(ion_uniform_block_);
    if (bc_index_ == kInvalidIndex)
        bc_index_ = AddIonUniform_(
            ion_registry_->Create<ion::gfx::Uniform>("uBaseColor", color));
    else
        ion_uniform_block_->SetUniformValue(bc_index_, color);
}

void UniformBlock::SetEmissiveColor(const Color &color) {
    ASSERT(ion_uniform_block_);
    if (ec_index_ == kInvalidIndex)
        ec_index_ = AddIonUniform_(ion_registry_->Create<ion::gfx::Uniform>(
                                       "uEmissiveColor", color));
    else
        ion_uniform_block_->SetUniformValue(ec_index_, color);
}

void UniformBlock::AddMaterialUniforms_(const Material &mat) {
    SetBaseColor(mat.GetBaseColor());
    SetEmissiveColor(mat.GetEmissiveColor());

    AddIonUniform_(ion_registry_->Create<ion::gfx::Uniform>(
                       "uSmoothness", mat.GetSmoothness()));
    AddIonUniform_(ion_registry_->Create<ion::gfx::Uniform>(
                       "uMetalness", mat.GetMetalness()));
}

void UniformBlock::AddTextureUniform_(const Texture &tex) {
    ASSERT(tex.GetIonTexture());

    const std::string &name  = tex.GetUniformName();
    const int          count = tex.GetCount();

    ion::gfx::Uniform u;
    if (count > 1) {
        std::vector<ion::gfx::TexturePtr> texvec(count, tex.GetIonTexture());
        u = ion_registry_->CreateArrayUniform(name, texvec.data(), count,
                                              ion::base::AllocatorPtr());
    }
    else {
        u = ion_registry_->Create<ion::gfx::Uniform>(name, tex.GetIonTexture());
    }
    AddIonUniform_(u);
}

size_t UniformBlock::AddIonUniform_(const ion::gfx::Uniform &uniform) {
    ASSERT(ion_uniform_block_);
    const size_t i = ion_uniform_block_->AddUniform(uniform);
    ASSERT(i != ion::base::kInvalidIndex);
    return i;
}

}  // namespace SG
