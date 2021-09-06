#include "SG/UniformBlock.h"

#include <ion/gfx/shaderinputregistry.h>

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
    if (mm_index_ < 0) {
        auto reg = ion::gfx::ShaderInputRegistry::GetGlobalRegistry();
        mm_index_ = ion_uniform_block_->AddUniform(
            reg->Create<ion::gfx::Uniform>("uModelMatrix", matrix));
        mv_index_ = ion_uniform_block_->AddUniform(
            reg->Create<ion::gfx::Uniform>("uModelviewMatrix", matrix));
    }
    else {
        ion_uniform_block_->SetUniformValue(mm_index_, matrix);
        ion_uniform_block_->SetUniformValue(mv_index_, matrix);
    }
}

void UniformBlock::SetUpIon(IonContext &context) {
    // Create the Ion UniformBlock if not already done.
    if (! ion_uniform_block_) {
        ion_uniform_block_.Reset(new ion::gfx::UniformBlock);
        ion_uniform_block_->SetLabel(GetName());
    }

    // Add uniforms that are valid for this pass. If this is a pass-specific
    // block, check the current pass. Otherwise, do this only once.
    if ((GetName().empty() && ! added_uniforms_) ||
        GetName() == context.pass_name) {

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
        added_uniforms_ = true;
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
