#include "SG/UniformBlock.h"

#include "Assert.h"
#include "Math/Types.h"

namespace SG {

void UniformBlock::AddFields() {
    AddField(pass_type_);
    AddField(material_);
    AddField(textures_);
    AddField(uniforms_);
}

void UniformBlock::SetModelMatrices(const Matrix4f &model_matrix,
                                    const Matrix4f &modelview_matrix) {
    ASSERT(ion_uniform_block_);

    // Create the uniforms if not already done.
    if (mm_index_ == kInvalidIndex) {
        mm_index_ = ion_uniform_block_->AddUniform(
            registry_->Create<ion::gfx::Uniform>("uModelMatrix", model_matrix));
        mv_index_ = ion_uniform_block_->AddUniform(
            registry_->Create<ion::gfx::Uniform>("uModelviewMatrix",
                                                 modelview_matrix));
        ASSERT(mm_index_ != ion::base::kInvalidIndex);
        ASSERT(mv_index_ != ion::base::kInvalidIndex);
    }
    else {
        ion_uniform_block_->SetUniformValue(mm_index_, model_matrix);
        ion_uniform_block_->SetUniformValue(mv_index_, modelview_matrix);
    }
}

void UniformBlock::SetBaseColor(const Color &color) {
    ASSERT(pass_type_ == PassType::kLightingPass);
    ASSERT(ion_uniform_block_);
    if (bc_index_ == kInvalidIndex) {
        bc_index_ = ion_uniform_block_->AddUniform(
            registry_->Create<ion::gfx::Uniform>("uBaseColor", color));
        ASSERT(bc_index_ != ion::base::kInvalidIndex);
    }
    else {
        ion_uniform_block_->SetUniformValue(bc_index_, color);
    }
}

void UniformBlock::SetEmissiveColor(const Color &color) {
    ASSERT(pass_type_ == PassType::kLightingPass);
    ASSERT(ion_uniform_block_);
    ASSERT(registry_ != ion::gfx::ShaderInputRegistry::GetGlobalRegistry());
    if (ec_index_ == kInvalidIndex) {
        ec_index_ = ion_uniform_block_->AddUniform(
            registry_->Create<ion::gfx::Uniform>("uEmissiveColor", color));
        ASSERT(ec_index_ != ion::base::kInvalidIndex);
    }
    else {
        ion_uniform_block_->SetUniformValue(ec_index_, color);
    }
}

void UniformBlock::SetUpIon(const ContextPtr &context) {
    Object::SetUpIon(context);

    // Create the Ion UniformBlock if not already done.
    if (! ion_uniform_block_) {
        ion_uniform_block_.Reset(new ion::gfx::UniformBlock);
        ion_uniform_block_->SetLabel(GetName());
    }

    // Add uniforms that are valid for this pass. If this is a pass-specific
    // block, check the current pass. Otherwise, do this only once.
    if (PassIn(context->pass_type, GetPassType()) && ! added_uniforms_) {
        // Save the registry from the context.
        registry_ = context->registry_stack.top();
        ASSERT(registry_);

        if (GetMaterial()) {
            AddMaterialUniforms_(*GetMaterial());
        }
        for (const auto &tex: GetTextures()) {
            tex->SetUpIon(context);
            AddTextureUniform_(*tex);
        }
        for (const auto &uni: GetUniforms()) {
            uni->SetUpIon(context);
            size_t i = ion_uniform_block_->AddUniform(uni->GetIonUniform());
            ASSERT(i != ion::base::kInvalidIndex);
        }
        added_uniforms_ = true;
    }
}

void UniformBlock::AddMaterialUniforms_(const Material &mat) {
    ASSERT(ion_uniform_block_);
    ASSERT(registry_);
    ASSERT(registry_ != ion::gfx::ShaderInputRegistry::GetGlobalRegistry());

    SetBaseColor(mat.GetBaseColor());
    SetEmissiveColor(mat.GetEmissiveColor());

    size_t si = ion_uniform_block_->AddUniform(
        registry_->Create<ion::gfx::Uniform>("uSmoothness",
                                             mat.GetSmoothness()));
    ASSERT(si != ion::base::kInvalidIndex);
    size_t mi = ion_uniform_block_->AddUniform(
        registry_->Create<ion::gfx::Uniform>("uMetalness", mat.GetMetalness()));
    ASSERT(mi != ion::base::kInvalidIndex);
}

void UniformBlock::AddTextureUniform_(const Texture &tex) {
    ASSERT(registry_);
    ASSERT(registry_ != ion::gfx::ShaderInputRegistry::GetGlobalRegistry());

    const std::string &name  = tex.GetUniformName();
    const int          count = tex.GetCount();

    ion::gfx::Uniform u;
    if (count > 1) {
        std::vector<ion::gfx::TexturePtr> texvec(count, tex.GetIonTexture());
        u = registry_->CreateArrayUniform(name, texvec.data(), count,
                                          ion::base::AllocatorPtr());
    }
    else {
        u = registry_->Create<ion::gfx::Uniform>(name, tex.GetIonTexture());
    }
    size_t ti = ion_uniform_block_->AddUniform(u);
    ASSERT(ti != ion::base::kInvalidIndex);
}

}  // namespace SG
