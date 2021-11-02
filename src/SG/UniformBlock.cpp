#include "SG/UniformBlock.h"

#include "Assert.h"
#include "Math/Types.h"
#include "Parser/Registry.h"

using ion::gfx::ShaderInputRegistry;

namespace SG {

void UniformBlock::AddFields() {
    AddField(pass_name_);
    AddField(material_);
    AddField(textures_);
    AddField(uniforms_);
    Object::AddFields();
}

void UniformBlock::SetPassName(const std::string &name) {
    ASSERT(GetPassName().empty());
    pass_name_ = name;
}

ion::gfx::UniformBlockPtr UniformBlock::SetUpIon(
    const IonContextPtr &ion_context,
    const ion::gfx::ShaderInputRegistryPtr &reg) {
    if (! ion_uniform_block_) {
        ion_uniform_block_.Reset(new ion::gfx::UniformBlock);
        ion_uniform_block_->SetLabel(GetName());

        ion_registry_ = reg;

        // Create Ion Textures, including their images and samplers, then add
        // them as uniforms.
        for (const auto &tex: GetTextures()) {
            tex->SetUpIon(ion_context);
            AddTextureUniform_(*tex);
        }

        // Add all other uniforms.
        if (GetMaterial())
            AddMaterialUniforms_(*GetMaterial());
        for (const auto &uniform: GetUniforms())
            AddIonUniform_(*uniform);
    }
    return ion_uniform_block_;
}

void UniformBlock::SetModelMatrices(const Matrix4f &model_matrix,
                                    const Matrix4f &modelview_matrix) {
    ASSERT(ion_uniform_block_);
    if (! mmu_) {
        mmu_ = CreateAndAddUniform_("uModelMatrix",     "mat4_val");
        mvu_ = CreateAndAddUniform_("uModelviewMatrix", "mat4_val");
    }
    ASSERT(mmu_->GetIonIndex() != ion::base::kInvalidIndex);
    ASSERT(mvu_->GetIonIndex() != ion::base::kInvalidIndex);
    mmu_->SetValue(model_matrix);
    mvu_->SetValue(modelview_matrix);
    ion_uniform_block_->SetUniformValue(mmu_->GetIonIndex(), model_matrix);
    ion_uniform_block_->SetUniformValue(mvu_->GetIonIndex(), modelview_matrix);
}

void UniformBlock::SetBaseColor(const Color &color) {
    ASSERT(ion_uniform_block_);
    if (! bcu_)
        bcu_ = CreateAndAddUniform_("uBaseColor", "vec4f_val");
    bcu_->SetValue<Vector4f>(color);
    ion_uniform_block_->SetUniformValue(bcu_->GetIonIndex(), color);
}

void UniformBlock::SetEmissiveColor(const Color &color) {
    ASSERT(ion_uniform_block_);
    if (! ecu_)
        ecu_ = CreateAndAddUniform_("uEmissiveColor", "vec4f_val");
    ecu_->SetValue<Vector4f>(color);
    ion_uniform_block_->SetUniformValue(ecu_->GetIonIndex(), color);
}

void UniformBlock::AddMaterialUniforms_(const Material &mat) {
    SetBaseColor(mat.GetBaseColor());
    SetEmissiveColor(mat.GetEmissiveColor());

    auto su = CreateAndAddUniform_("uSmoothness", "float_val");
    auto mu = CreateAndAddUniform_("uMetalness",  "float_val");
    su->SetValue(mat.GetSmoothness());
    mu->SetValue(mat.GetMetalness());
    ion_uniform_block_->SetUniformValue(su->GetIonIndex(), mat.GetSmoothness());
    ion_uniform_block_->SetUniformValue(mu->GetIonIndex(), mat.GetMetalness());
}

void UniformBlock::AddTextureUniform_(const Texture &tex) {
    ASSERT(ion_uniform_block_);
    ASSERT(tex.GetIonTexture());

    // Bypass the SG::Uniform code, since there is no version for textures.
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
    ion_uniform_block_->AddUniform(u);
}

UniformPtr UniformBlock::CreateAndAddUniform_(const std::string &name,
                                              const std::string &field_name) {
    ASSERT(ion_uniform_block_);
    UniformPtr u = Parser::Registry::CreateObject<Uniform>(name);
    u->SetFieldName(field_name);
    uniforms_.Add(u);
    AddIonUniform_(*u);
    return u;
}

void UniformBlock::AddIonUniform_(Uniform &uniform) {
    ASSERT(ion_uniform_block_);
   uniform.SetUpIon(*ion_registry_, *ion_uniform_block_);
    ASSERTM(uniform.GetIonIndex() != ion::base::kInvalidIndex,
            uniform.GetDesc());
}

}  // namespace SG
