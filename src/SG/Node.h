#pragma once

#include <string>
#include <vector>

#include <ion/gfx/node.h>
#include <ion/gfx/shaderprogram.h>
#include <ion/gfx/statetable.h>
#include <ion/gfx/uniform.h>

#include "NParser/FieldSpec.h"
#include "SG/Math.h"
#include "SG/Object.h"
#include "SG/Typedefs.h"

namespace SG {

//! The Node class represents the main type of object constructing a scene
//! graph.  It contains an Ion Node.
//!
//! \ingroup SG
class Node : public Object {
  public:
    Node();
    virtual ~Node();

    //! Returns the associated Ion node.
    const ion::gfx::NodePtr &GetIonNode() { return i_node_; }

    //! Enables or disables the node.
    void SetEnabled(bool enabled);

    //! Returns true if the node is enabled.
    bool IsEnabled() const;

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

    //! Returns the shader program in the node.
    const ShaderProgramPtr & GetShaderProgram() const {
        return shader_program_;
    }
    //! Returns the textures in the node.
    const std::vector<TexturePtr> & GetTextures() const { return textures_; }
    //! Returns the shapes in the node.
    const std::vector<ShapePtr>   & GetShapes()   const { return shapes_;   }
    //! Returns the child nodes in the node.
    const std::vector<NodePtr>    & GetChildren() const { return children_; }

    static std::vector<NParser::FieldSpec> GetFieldSpecs();

  private:
    Vector3f  scale_{ 1, 1, 1 };        //!< Scale component.
    Rotationf rotation_;                //!< Rotation component.
    Vector3f  translation_{ 0, 0, 0 };  //!< Translation component.

    ion::gfx::NodePtr       i_node_;  //! Associated Ion Node.

    ShaderProgramPtr        shader_program_;
    std::vector<TexturePtr> textures_;
    std::vector<ShapePtr>   shapes_;
    std::vector<NodePtr>    children_;

    //! Uniform index for uModelviewMatrix.
    int matrix_index_ = -1;

    void SetScale_(const ion::math::Vector3f &scale);
    void SetRotation_(const ion::math::Rotationf &rotation);
    void SetTranslation_(const ion::math::Vector3f &translation);

    void SetStateTable_(const ion::gfx::StateTablePtr &state_table);
    void SetShaderProgram_(const ShaderProgramPtr &program);

    //! Returns the uniform index.
    int AddUniform_(const ion::gfx::Uniform &uniform);

    //! Adds a texture.
    void AddTexture_(const TexturePtr &texture);

    //! Adds a shape.
    void AddShape_(const ShapePtr &shape);

    //! Clears the list of child nodes.
    void ClearChildren_();

    //! Adds a child.
    void AddChild_(const NodePtr &child);

    //! Updates the uModelviewMatrix uniform when some transformation field
    //! changes.
    void UpdateMatrix_();
};

}  // namespace SG
