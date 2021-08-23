#pragma once

#include <string>
#include <vector>

#include <ion/gfx/node.h>

#include "Parser/ObjectSpec.h"
#include "SG/Math.h"
#include "SG/Object.h"
#include "SG/Typedefs.h"
#include "Util/Enum.h"

namespace SG {

//! The Node class represents the main type of object constructing a scene
//! graph.  It contains an Ion Node.
class Node : public Object {
  public:
    //! Flags defining the Node's behavior. Flags that disable behavior apply
    //! to the Node and the subgraph below it. Defaults are all true.
    enum class Flag : uint32_t {
        kRender      = (1 << 0),  //!< Enabled for rendering.
        kIntersect   = (1 << 1),  //!< Enabled for intersection testing.
        kCastShadows = (1 << 2),  //!< Casts shadows on other objects.
    };

    Node();

    //! Returns the associated Ion node.
    const ion::gfx::NodePtr &GetIonNode() { return ion_node_; }

    //! Enables or disables a node flag.
    void SetFlag(Flag flag, bool b) {
        if (b)
            flags_.Set(flag);
        else
            flags_.Reset(flag);
    }

    //! Enables or disables all node flags.
    void SetAllFlags(bool b) { flags_.SetAll(b); }

    //! Returns true if the given flag is enabled.
    bool IsFlagSet(Flag flag) const { return flags_.Has(flag); }

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

    virtual void SetUpIon(IonContext &context) override;

    static Parser::ObjectSpec GetObjectSpec();

  protected:
    //! Allows derived classes to set the Ion Node.
    void SetIonNode(const ion::gfx::NodePtr &node) { ion_node_ = node; }

  private:
    ion::gfx::NodePtr ion_node_;  //! Associated Ion Node.

    //! Flags defining behavior.
    Util::Flags<Flag>    flags_ ;

    // Parsed fields.
    bool                    casts_shadows_ = true;
    Vector3f                scale_{ 1, 1, 1 };
    Rotationf               rotation_;
    Vector3f                translation_{ 0, 0, 0 };
    StateTablePtr           state_table_;
    ShaderProgramPtr        shader_program_;
    std::vector<TexturePtr> textures_;
    std::vector<UniformPtr> uniforms_;
    std::vector<ShapePtr>   shapes_;
    std::vector<NodePtr>    children_;

    //! Uniform index for uModelviewMatrix.
    int matrix_index_ = -1;

    //! Updates the uModelviewMatrix uniform when some transformation field
    //! changes.
    void UpdateMatrix_();
};

}  // namespace SG
