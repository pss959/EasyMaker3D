#pragma once

#include <ion/gfx/shaderinputregistry.h>
#include <ion/gfx/uniformblock.h>

#include "Math/Types.h"
#include "Memory.h"
#include "SG/Material.h"
#include "SG/Object.h"
#include "SG/Texture.h"
#include "SG/Uniform.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(IonContext);
DECL_SHARED_PTR(UniformBlock);

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

    /// Sets the name of the sub-image to access in each of the textures. If no
    /// name is specified, the entire texture image is used.
    void SetSubImageName(const std::string &name);

    /// Returns the name of the sub-image to access in each of the textures. If
    /// no name is specified, the entire texture image is used.
    const std::string & GetSubImageName() const { return sub_image_name_; }

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

    /// Special case for setting the value of some other float Uniform in the
    /// block. The Uniform must already exist in the block.
    void SetFloatUniformValue(const std::string &name, float value);

    /// Special case for setting the value of some other Vector3f Uniform in
    /// the block. The Uniform must already exist in the block.
    void SetVector3fUniformValue(const std::string &name,
                                 const Vector3f &value);

    /// Special case for setting the value of some other Matrix4f Uniform in
    /// the block. The Uniform must already exist in the block.
    void SetMatrix4fUniformValue(const std::string &name,
                                 const Matrix4f &value);

  protected:
    UniformBlock() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string>      pass_name_{"pass_name"};
    Parser::ObjectField<Material>    material_{"material"};
    Parser::ObjectListField<Texture> textures_{"textures"};
    Parser::TField<std::string>      sub_image_name_{"sub_image_name"};
    Parser::ObjectListField<Uniform> uniforms_{"uniforms"};
    ///@}

    /// Associated Ion UniformBlock.
    ion::gfx::UniformBlockPtr ion_uniform_block_;

    /// Ion ShaderInputRegistry used to create and add uniforms. This is passed
    /// into SetUpIon().
    ion::gfx::ShaderInputRegistryPtr ion_registry_;

    /// \name Special Uniforms
    ///@{
    UniformPtr mmu_;   ///< Uniform for uModelMatrix.
    UniformPtr mvu_;   ///< Uniform for uModelviewMatrix.
    UniformPtr bcu_;   ///< Uniform for uBaseColor.
    UniformPtr ecu_;   ///< Uniform for uEmissiveColor.
    ///@}

    /// Adds Ion Uniforms for the given Material.
    void AddMaterialUniforms_(const Material &mat);

    /// Adds an Ion Uniform for the given Texture.
    void AddTextureUniform_(const Texture &tex);

    /// Updates the sub-image texture scale and offset in the given Texture,
    /// creating the Ion uniforms first if necessary.
    void UpdateSubImage_(const Texture &tex);

    /// Creates a Uniform with the given name and field name and adds its Ion
    /// Uniform to the block. Returns the new Uniform.
    UniformPtr CreateAndAddUniform_(const std::string &name,
                                    const std::string &field_name);

    /// Adds the given Uniform to the Ion UniformBlock. If it is one of the
    /// special uniforms, this saves it.
    void AddIonUniform_(const UniformPtr &uniform);

    friend class Parser::Registry;
};

}  // namespace SG
