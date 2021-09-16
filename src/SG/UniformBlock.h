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
//! If A UniformBlock has a name, it corresponds to the name of the RenderPass
//! that it is valid for. Because of this behavior, using a UniformBlock
//! instead of loose Uniforms makes it much easier for render-pass-specific
//! uniforms to be set.
class UniformBlock : public Object {
  public:
    //! Default constructor.
    UniformBlock() {}

    //! Constructor that sets the name of the UniformBlock for a specific
    //! render pass.
    UniformBlock(const std::string &name) { SetName(name); }

    virtual void AddFields() override;

    //! Returns the associated Ion UniformBlock. This will be null until
    //! CreateIonUniformBlock() is called.
    const ion::gfx::UniformBlockPtr & GetIonUniformBlock() const {
        return ion_uniform_block_;
    }

    //! Creates and stores an empty Ion UniformBlock.
    void CreateIonUniformBlock();

    //! Sets the Ion registry used to create uniforms. If this is never called,
    //! the global registry is assumed.
    void SetIonRegistry(const ion::gfx::ShaderInputRegistryPtr &registry) {
        ion_registry_ = registry;
    }

    //! Creates and stores Ion Uniforms in the Ion UniformBlock using the
    //! registry. CreateIonUniformBlock() must have been called.
    void AddIonUniforms();

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

  private:
    const size_t kInvalidIndex = ion::base::kInvalidIndex;

    //! \name Parsed Fields
    //!@{
    Parser::ObjectField<Material>    material_{"material"};
    Parser::ObjectListField<Texture> textures_{"textures"};
    Parser::ObjectListField<Uniform> uniforms_{"uniforms"};
    //!@}

    //! Associated Ion UniformBlock.
    ion::gfx::UniformBlockPtr ion_uniform_block_;

    //! Ion ShaderInputRegistry used to create and add uniforms. This is
    //! assumed to be the global registry until SetIonRegistry() is called.
    ion::gfx::ShaderInputRegistryPtr ion_registry_ =
        ion::gfx::ShaderInputRegistry::GetGlobalRegistry();

    //! \name Special Uniform Indices
    //!@{
    size_t mm_index_ = kInvalidIndex;  //!< Uniform index for uModelMatrix.
    size_t mv_index_ = kInvalidIndex;  //!< Uniform index for uModelviewMatrix.
    size_t bc_index_ = kInvalidIndex;  //!< Uniform index for uBaseColor.
    size_t ec_index_ = kInvalidIndex;  //!< Uniform index for uEmissiveColor.
    //!@}

    //! Adds Ion Uniforms for the given Material.
    void AddMaterialUniforms_(const Material &mat);

    //! Adds an Ion Uniform for the given Texture.
    void AddTextureUniform_(const Texture &tex);

    //! Adds the given Ion Uniform to the Ion UniformBlock, asserting that it
    //! succeeded. Returns the resulting index.
    size_t AddIonUniform_(const ion::gfx::Uniform &uniform);
};

}  // namespace SG
