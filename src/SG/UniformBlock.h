#pragma once

#include <ion/base/invalid.h>
#include <ion/gfx/shaderinputregistry.h>
#include <ion/gfx/uniformblock.h>

#include "Math/Types.h"
#include "SG/IonContext.h"
#include "SG/Material.h"
#include "SG/Object.h"
#include "SG/Texture.h"
#include "SG/Uniform.h"

namespace Parser { class Registry; }

namespace SG {

/// A UniformBlock object wraps an Ion UniformBlock. All uniforms in Nodes are
/// stored in UniformBlock instances. Material and Texture uniforms are stored
/// separately, as Material and Texture instances.
///
/// If a UniformBlock has a non-empty pass_name field, it corresponds to the
/// name of the RenderPass that it is valid for; otherwise, it must be valid
/// for all RenderPasses (using the global registry). Because of this feature,
/// using a UniformBlock instead of loose Uniforms makes it much easier for
/// render-pass-specific uniforms to be set.
class UniformBlock : public Object {
  public:
    virtual void AddFields() override;

    /// Sets the render pass name for this UniformBlock. The default name is
    /// empty, meaning that it is valid for all render passes.
    void SetPassName(const std::string &name);

    /// Returns the render pass name for this UniformBlock. If it is empty, it
    /// is valid for all render passes.
    const std::string & GetPassName() const { return pass_name_; }

    /// Returns the Material in the UniformBlock.
    const MaterialPtr & GetMaterial() const { return material_; }

    /// Returns the textures in the UniformBlock.
    const std::vector<TexturePtr> & GetTextures() const { return textures_; }

    /// Returns the non-texture uniforms in the UniformBlock.
    const std::vector<UniformPtr> & GetUniforms() const { return uniforms_; }

    /// Creates, stores, and returns the Ion UniformBlock. This is passed the
    /// IonContext and Ion registry to use for creating uniforms.
    ion::gfx::UniformBlockPtr SetUpIon(
        const IonContextPtr &ion_context,
        const ion::gfx::ShaderInputRegistryPtr &reg);

    /// Returns the associated Ion UniformBlock. This will be null until
    /// SetUpIon() is called.
    const ion::gfx::UniformBlockPtr & GetIonUniformBlock() const {
        return ion_uniform_block_;
    }

    /// Special-case for setting uModelMatrix and uModelviewMatrix in the
    /// UniformBlock, creating them first if necessary. This works only after
    /// SetUpIon() is called.
    void SetModelMatrices(const Matrix4f &model_matrix,
                          const Matrix4f &modelview_matrix);

    /// Special-case for setting uBaseColor in the UniformBlock. This works
    /// only after SetUpIon() is called.
    void SetBaseColor(const Color &color);

    /// Special-case for setting uEmissiveColor in the UniformBlock. This works
    /// only after SetUpIon() is called.
    void SetEmissiveColor(const Color &color);

  protected:
    UniformBlock() {}

  private:
    const size_t kInvalidIndex = ion::base::kInvalidIndex;

    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string>      pass_name_{"pass_name"};
    Parser::ObjectField<Material>    material_{"material"};
    Parser::ObjectListField<Texture> textures_{"textures"};
    Parser::ObjectListField<Uniform> uniforms_{"uniforms"};
    ///@}

    /// Associated Ion UniformBlock.
    ion::gfx::UniformBlockPtr ion_uniform_block_;

    /// Ion ShaderInputRegistry used to create and add uniforms. This is passed
    /// into SetUpIon().
    ion::gfx::ShaderInputRegistryPtr ion_registry_;

    /// \name Special Uniform Indices
    ///@{
    size_t mm_index_ = kInvalidIndex;  ///< Uniform index for uModelMatrix.
    size_t mv_index_ = kInvalidIndex;  ///< Uniform index for uModelviewMatrix.
    size_t bc_index_ = kInvalidIndex;  ///< Uniform index for uBaseColor.
    size_t ec_index_ = kInvalidIndex;  ///< Uniform index for uEmissiveColor.
    ///@}

    /// Adds Ion Uniforms for the given Material.
    void AddMaterialUniforms_(const Material &mat);

    /// Adds an Ion Uniform for the given Texture.
    void AddTextureUniform_(const Texture &tex);

    /// Adds the given Ion Uniform to the Ion UniformBlock, asserting that it
    /// succeeded. Returns the resulting index.
    size_t AddIonUniform_(const ion::gfx::Uniform &uniform);

    friend class Parser::Registry;
};

}  // namespace SG
