#pragma once

#include <string>
#include <vector>

#include <ion/gfx/node.h>

#include "Parser/ObjectSpec.h"
#include "SG/Math.h"
#include "SG/Object.h"
#include "SG/Typedefs.h"
#include "Util/Flags.h"

namespace SG {

//! The Node class represents the main type of object constructing a scene
//! graph.  It contains an Ion Node.
class Node : public Object {
  public:
    //! Flags used to enabled or disable specific Node behavior; they apply to
    //! the Node and the subgraph below it. Defaults are all false, meaning
    //! that nothing is disabled. Note that disabling the Flag::kTraversal flag
    //! effectively disables all the others.
    enum class Flag : uint32_t {
        kTraversal   = (1 << 0),  //!< Disable all traversals.
        kRender      = (1 << 1),  //!< Disable rendering.
        kIntersect   = (1 << 2),  //!< Disable intersection testing.
        kCastShadows = (1 << 3),  //!< Disable casting shadows on other objects.
    };

    //! Returns the associated Ion node.
    const ion::gfx::NodePtr &GetIonNode() { return ion_node_; }

    //! Enables or disables the node behavior(s) associated with a DisableFlag.
    void SetEnabled(Flag flag, bool b) {
        // Inverse setting, since flags indicate what is disabled.
        if (b)
            disabled_flags_.Reset(flag);
        else
            disabled_flags_.Set(flag);
    }

    //! Returns true if the given behavior is enabled.
    bool IsEnabled(Flag flag) const { return ! disabled_flags_.Has(flag); }

    //! Returns the set of disabled flags.
    const Util::Flags<Flag> & GetDisabledFlags() const {
        return disabled_flags_;
    }

    //! \name Transformation Modification Functions.
    //! Each of these updates the Node and its Ion Matrix uniform.
    //!@{
    void SetScale(const Vector3f &scale);
    void SetRotation(const Rotationf &rotation);
    void SetTranslation(const Vector3f &translation);
    //!@}

    //! \name Transformation Query Functions.
    //!@{
    const Vector3f  & GetScale()       const { return scale_;       }
    const Rotationf & GetRotation()    const { return rotation_;    }
    const Vector3f  & GetTranslation() const { return translation_; }
    //!@}

    //! Returns the state table in the node.
    const StateTablePtr & GetStateTable() const { return state_table_; }
    //! Returns the shader program in the node.
    const ShaderProgramPtr & GetShaderProgram() const {
        return shader_program_;
    }
    //! Returns the textures in the node.
    const std::vector<TexturePtr> & GetTextures() const { return textures_; }
    //! Returns the other uniforms in the node.
    const std::vector<UniformPtr> & GetUniforms() const { return uniforms_; }
    //! Returns the shapes in the node.
    const std::vector<ShapePtr>   & GetShapes()   const { return shapes_;   }
    //! Returns the child nodes in the node.
    const std::vector<NodePtr>    & GetChildren() const { return children_; }

    //! Updates all state in the Node if necessary.
    void Update();

    virtual void SetUpIon(IonContext &context) override;

    static Parser::ObjectSpec GetObjectSpec();

  protected:
    //! Allows derived classes to set the Ion Node.
    void SetIonNode(const ion::gfx::NodePtr &node) { ion_node_ = node; }

  private:
    ion::gfx::NodePtr ion_node_;  //! Associated Ion Node.

    // Parsed fields.
    Util::Flags<Flag>       disabled_flags_;
    Vector3f                scale_{ 1, 1, 1 };
    Rotationf               rotation_;
    Vector3f                translation_{ 0, 0, 0 };
    StateTablePtr           state_table_;
    ShaderProgramPtr        shader_program_;
    std::vector<TexturePtr> textures_;
    std::vector<UniformPtr> uniforms_;
    std::vector<ShapePtr>   shapes_;
    std::vector<NodePtr>    children_;

    bool need_to_update_matrices_ = false;

    int mm_index_ = -1;   //! Uniform index for uModelMatrix.
    int mv_index_ = -1;   //! Uniform index for uModelviewMatrix.

    //! Adds an Ion Uniform for the given Texture.
    void AddTextureUniform_(IonContext &context, const Texture &tex);

    //! Updates the uModelviewMatrix and uNormalMatrix uniforms when some
    //! transformation field changes.
    void UpdateMatrices_();
};

}  // namespace SG
