#pragma once

#include <ion/gfx/uniformblock.h>

#include "Math/Types.h"
#include "SG/Material.h"
#include "SG/Object.h"
#include "SG/Texture.h"
#include "SG/Uniform.h"

namespace SG {

//! A UniformBlock object wraps an Ion UniformBlock. All uniforms in Nodes are
//! stored in UniformBlock instances. Material and Texture uniforms are stored
//! separately, as Material and Texture instances.
//!
//! Rationale for using UniformBlock instances instead of just uniforms:
//! Storing uniforms directly in nodes means that they will always be sent,
//! even if the current rendering pass's shader does not support them. If a
//! UniformBlock has a name, it indicates which RenderPass the block should be
//! enabled for. This allows the same Node to set different uniforms based on
//! the current RenderPass.
class UniformBlock : public Object {
  public:
    virtual void AddFields() override;

    //! Returns the associated Ion UniformBlock.
    const ion::gfx::UniformBlockPtr & GetIonUniformBlock() const {
        return ion_uniform_block_;
    }

    //! Returns the Material in the UniformBlock.
    const MaterialPtr & GetMaterial() const { return material_; }

    //! Returns the textures in the UniformBlock.
    const std::vector<TexturePtr> & GetTextures() const { return textures_; }

    //! Returns the non-texture uniforms in the UniformBlock.
    const std::vector<UniformPtr> & GetUniforms() const { return uniforms_; }

    //! Special-case for setting uModelMatrix and uModelviewMatrix in the
    //! UniformBlock, creating them first if necessary. This should work for
    //! all render passes.
    void SetModelMatrices(const Matrix4f &matrix);

    virtual void SetUpIon(IonContext &context) override;

  private:
    //! Associated Ion UniformBlock.
    ion::gfx::UniformBlockPtr ion_uniform_block_;

    //! \name Parsed Fields
    //!@{
    Parser::ObjectField<Material>    material_{"material"};
    Parser::ObjectListField<Texture> textures_{"textures"};
    Parser::ObjectListField<Uniform> uniforms_{"uniforms"};
    //!@}

    int mm_index_ = -1;   //! Uniform index for uModelMatrix.
    int mv_index_ = -1;   //! Uniform index for uModelviewMatrix.

    //! Set to true after uniforms are added in at least one pass.
    bool added_uniforms_ = false;

    //! Adds Ion Uniforms for the given Material.
    void AddMaterialUniforms_(IonContext &context, const Material &mat);

    //! Adds an Ion Uniform for the given Texture.
    void AddTextureUniform_(IonContext &context, const Texture &tex);
};

}  // namespace SG
