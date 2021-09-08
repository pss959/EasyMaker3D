#include "SG/UniformBlock.h"

#include "Assert.h"
#include "Math/Types.h"

namespace SG {

void UniformBlock::AddFields() {
    AddField(material_);
    AddField(textures_);
    AddField(uniforms_);
}

void UniformBlock::SetModelMatrices(const Matrix4f &matrix) {
    ASSERT(ion_uniform_block_);

    // Create the uniforms if not already done.
    if (mm_index_ == kInvalidIndex) {
        mm_index_ = ion_uniform_block_->AddUniform(
            registry_->Create<ion::gfx::Uniform>("uModelMatrix", matrix));
        mv_index_ = ion_uniform_block_->AddUniform(
            registry_->Create<ion::gfx::Uniform>("uModelviewMatrix", matrix));
        ASSERT(mm_index_ != ion::base::kInvalidIndex);
        ASSERT(mv_index_ != ion::base::kInvalidIndex);
    }
    else {
        ion_uniform_block_->SetUniformValue(mm_index_, matrix);
        ion_uniform_block_->SetUniformValue(mv_index_, matrix);
    }
}

void UniformBlock::SetBaseColor(const Color &color) {
    ASSERT(GetName() == "Lighting Pass");
    ASSERT(ion_uniform_block_);
    if (bc_index_ == kInvalidIndex) {
        bc_index_ = ion_uniform_block_->AddUniform(
            registry_->Create<ion::gfx::Uniform>("uEmissiveColor", color));
        ASSERT(bc_index_ != ion::base::kInvalidIndex);
    }
    else {
        ion_uniform_block_->SetUniformValue(bc_index_, color);
    }
}

void UniformBlock::SetEmissiveColor(const Color &color) {
    ASSERT(GetName() == "Lighting Pass");
    ASSERT(ion_uniform_block_);
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
    if ((GetName().empty() && ! added_uniforms_) ||
        GetName() == context->pass_name) {

        // Save the registry from the context.
        registry_ = context->registry_stack.top();
        ASSERT(registry_);

        if (GetMaterial()) {
            AddMaterialUniforms_(*context, *GetMaterial());
        }
        for (const auto &tex: GetTextures()) {
            tex->SetUpIon(context);
            AddTextureUniform_(*context, *tex);
        }
        for (const auto &uni: GetUniforms()) {
            uni->SetUpIon(context);
            size_t i = ion_uniform_block_->AddUniform(uni->GetIonUniform());
            ASSERT(i != ion::base::kInvalidIndex);
        }
        added_uniforms_ = true;
    }
}

void UniformBlock::AddMaterialUniforms_(Context &context, const Material &mat) {
    ASSERT(ion_uniform_block_);
    ASSERT(registry_);

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

void UniformBlock::AddTextureUniform_(Context &context, const Texture &tex) {
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
    size_t ti = ion_uniform_block_->AddUniform(u);
    ASSERT(ti != ion::base::kInvalidIndex);
}

}  // namespace SG
