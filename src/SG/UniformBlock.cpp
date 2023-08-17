#include "SG/UniformBlock.h"

#include <ion/base/invalid.h>

#include "Math/Types.h"
#include "Parser/Registry.h"
#include "Util/Assert.h"

using ion::gfx::ShaderInputRegistry;

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

namespace {

/// Looks for a an SG::Uniform with the given name in the vector and sets its
/// value.
template <typename T>
static void SetUniformValue_(const std::vector<SG::UniformPtr> &uniforms,
                             ion::gfx::UniformBlockPtr ion_block,
                             const Str &name, const T &value) {
    ASSERT(ion_block);

    for (const auto &uniform: uniforms) {
        if (uniform->GetName() == name) {
            uniform->SetValue<T>(value);
            ion_block->SetUniformValue(uniform->GetIonIndex(), value);
            return;
        }
    }
    ASSERTM(false, "No such uniform " + name);
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// UniformBlock functions.
// ----------------------------------------------------------------------------

namespace SG {

void UniformBlock::AddFields() {
    AddField(pass_name_.Init("pass_name"));
    AddField(material_.Init("material"));
    AddField(textures_.Init("textures"));
    AddField(sub_image_name_.Init("sub_image_name"));
    AddField(uniforms_.Init("uniforms"));

    Object::AddFields();
}

void UniformBlock::SetPassName(const Str &name) {
    ASSERT(GetPassName().empty());
    pass_name_ = name;
}

void UniformBlock::SetSubImageName(const Str &name) {
    sub_image_name_ = name;
    if (ion_uniform_block_) {
        for (const auto &tex: GetTextures())
            UpdateSubImage_(*tex);
    }

    ProcessChange(Change::kAppearance, *this);
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

void UniformBlock::SetIntUniformValue(const Str &name, int value) {
    SetUniformValue_<int>(GetUniforms(), ion_uniform_block_, name, value);
}

void UniformBlock::SetFloatUniformValue(const Str &name, float value) {
    SetUniformValue_<float>(GetUniforms(), ion_uniform_block_, name, value);
}

void UniformBlock::SetVector3fUniformValue(const Str &name,
                                           const Vector3f &value) {
    SetUniformValue_<Vector3f>(GetUniforms(), ion_uniform_block_, name, value);
}

void UniformBlock::SetVector4fUniformValue(const Str &name,
                                           const Vector4f &value) {
    SetUniformValue_<Vector4f>(GetUniforms(), ion_uniform_block_, name, value);
}

void UniformBlock::SetMatrix4fUniformValue(const Str &name,
                                           const Matrix4f &value) {
    SetUniformValue_<Matrix4f>(GetUniforms(), ion_uniform_block_, name, value);
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
    const Str &name  = tex.GetUniformName();
    const int  count = tex.GetCount();
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
    if (! GetSubImageName().empty())
        UpdateSubImage_(tex);
}

void UniformBlock::UpdateSubImage_(const Texture &tex) {
    ASSERT(ion_uniform_block_);
    ASSERT(tex.GetImage());
    const Str &sub_image_name = GetSubImageName();
    const auto sub = tex.GetImage()->FindSubImage(sub_image_name);
    ASSERTM(sub, "SubImage '" + sub_image_name + "'");

    // If the Ion uniforms do not already exist, create and add them. Note that
    // there is no SG equivalent needed for these uniforms, so they are just
    // handled directly here. Also note that this is a rare change, so there is
    // no need to save the index.
    auto set_uniform = [&](const Str &name, const Vector2f &value){
        const size_t index = ion_uniform_block_->GetUniformIndex(name);
        if (index == ion::base::kInvalidIndex)
            ion_uniform_block_->AddUniform(
                ion_registry_->Create<ion::gfx::Uniform>(name, value));
        else
            ion_uniform_block_->SetUniformValue(index, value);
    };
    set_uniform("uTextureScale",  sub->GetTextureScale());
    set_uniform("uTextureOffset", sub->GetTextureOffset());
}

UniformPtr UniformBlock::CreateAndAddUniform_(const Str &name,
                                              const Str &field_name) {
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
