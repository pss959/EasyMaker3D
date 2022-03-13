#include "SG/UniformBlock.h"

#include <ion/base/invalid.h>

#include "Math/Types.h"
#include "Parser/Registry.h"
#include "Util/Assert.h"

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
            AddIonUniform_(uniform);
    }
    return ion_uniform_block_;
}

void UniformBlock::SetModelMatrices(const Matrix4f &model_matrix,
                                    const Matrix4f &modelview_matrix) {
    ASSERT(ion_uniform_block_);
    if (! mmu_)
        CreateAndAddUniform_("uModelMatrix", "mat4_val");
    if (! mvu_)
        CreateAndAddUniform_("uModelviewMatrix", "mat4_val");
    ASSERT(mmu_);
    ASSERT(mvu_);
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
        CreateAndAddUniform_("uBaseColor", "vec4f_val");
    ASSERT(bcu_);
    bcu_->SetValue<Vector4f>(color);
    ion_uniform_block_->SetUniformValue(bcu_->GetIonIndex(), color);
}

void UniformBlock::SetEmissiveColor(const Color &color) {
    ASSERT(ion_uniform_block_);
    if (! ecu_)
        CreateAndAddUniform_("uEmissiveColor", "vec4f_val");
    ASSERT(ecu_);
    ecu_->SetValue<Vector4f>(color);
    ion_uniform_block_->SetUniformValue(ecu_->GetIonIndex(), color);
}

void UniformBlock::SetFloatUniformValue(const std::string &name, float value) {
    ASSERT(ion_uniform_block_);

    for (const auto &uniform: GetUniforms()) {
        if (uniform->GetName() == name) {
            uniform->SetValue<float>(value);
            ion_uniform_block_->SetUniformValue(uniform->GetIonIndex(), value);
            return;
        }
    }
    ASSERTM(false, "No such uniform " + name + " in " + GetDesc());
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

    // If a sub-image is specified, set the texture scale and offset uniforms.
    const std::string &sub_image_name = tex.GetSubImageName();
    if (! sub_image_name.empty()) {
        ASSERT(tex.GetImage());
        const auto sub = tex.GetImage()->FindSubImage(sub_image_name);
        ASSERTM(sub, "SubImage '" + sub_image_name + "'");
        u = ion_registry_->Create<ion::gfx::Uniform>("uTextureScale",
                                                     sub->GetTextureScale());
        ion_uniform_block_->AddUniform(u);
        u = ion_registry_->Create<ion::gfx::Uniform>("uTextureOffset",
                                                     sub->GetTextureOffset());
        ion_uniform_block_->AddUniform(u);
    }
}

UniformPtr UniformBlock::CreateAndAddUniform_(const std::string &name,
                                              const std::string &field_name) {
    ASSERT(ion_uniform_block_);
    UniformPtr u = Parser::Registry::CreateObject<Uniform>(name);
    u->SetFieldName(field_name);
    uniforms_.Add(u);
    AddIonUniform_(u);
    return u;
}

void UniformBlock::AddIonUniform_(const UniformPtr &uniform) {
    ASSERT(uniform);
    ASSERT(ion_uniform_block_);
    uniform->SetUpIon(*ion_registry_, *ion_uniform_block_);
    ASSERTM(uniform->GetIonIndex() !=
            ion::base::kInvalidIndex, uniform->GetDesc());

    // Save important uniforms.
    if (uniform->GetName() == "uModelMatrix")
        mmu_ = uniform;
    else if (uniform->GetName() == "uModelviewMatrix")
        mvu_ = uniform;
    else if (uniform->GetName() == "uBaseColor")
        bcu_ = uniform;
    else if (uniform->GetName() == "uEmissiveColor")
        ecu_ = uniform;
}

}  // namespace SG
