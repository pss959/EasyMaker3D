#pragma once

#include <ion/base/invalid.h>
#include <ion/gfx/shaderinputregistry.h>
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
    //! Default constructor.
    UniformBlock() {}
    //! Constructor that sets the name to the given render pass name.
    UniformBlock(const std::string &pass_name) { SetName(pass_name); }

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
    void SetModelMatrices(const Matrix4f &model_matrix,
                          const Matrix4f &modelview_matrix);

    //! Special-case for setting uBaseColor in the UniformBlock. This works
    //! only in the lighting pass.
    void SetBaseColor(const Color &color);

    //! Special-case for setting uEmissiveColor in the UniformBlock. This works
    //! only in the lighting pass.
    void SetEmissiveColor(const Color &color);

    virtual void SetUpIon(const ContextPtr &context) override;

  private:
    const size_t kInvalidIndex = ion::base::kInvalidIndex;

    //! Associated Ion UniformBlock.
    ion::gfx::UniformBlockPtr ion_uniform_block_;

    //! Saves the Ion ShaderInputRegistry in effect when this UniformBlock was
    //! set up.
    ion::gfx::ShaderInputRegistryPtr registry_;

    //! \name Parsed Fields
    //!@{
    Parser::ObjectField<Material>    material_{"material"};
    Parser::ObjectListField<Texture> textures_{"textures"};
    Parser::ObjectListField<Uniform> uniforms_{"uniforms"};
    //!@}

    //! \name Special Uniform Indices
    //!@{
    size_t mm_index_ = kInvalidIndex;  //!< Uniform index for uModelMatrix.
    size_t mv_index_ = kInvalidIndex;  //!< Uniform index for uModelviewMatrix.
    size_t bc_index_ = kInvalidIndex;  //!< Uniform index for uBaseColor.
    size_t ec_index_ = kInvalidIndex;  //!< Uniform index for uEmissiveColor.
    //!@}

    //! Set to true after uniforms are added in at least one pass.
    bool added_uniforms_ = false;

    //! Adds Ion Uniforms for the given Material.
    void AddMaterialUniforms_(Context &context, const Material &mat);

    //! Adds an Ion Uniform for the given Texture.
    void AddTextureUniform_(Context &context, const Texture &tex);
};

}  // namespace SG
