#pragma once

#include <vector>

#include <ion/gfx/node.h>
#include <ion/gfx/shaderprogram.h>
#include <ion/gfx/statetable.h>
#include <ion/math/rotation.h>
#include <ion/math/vector.h>

#include "Graph/Object.h"
#include "Graph/Transform.h"
#include "Graph/Typedefs.h"

namespace Graph {

//! The Node class represents the main type of object constructing a Graph.  It
//! wraps an Ion Node.
//! \ingroup Graph
class Node : public Object {
  public:
    //! Returns the associated Ion node.
    const ion::gfx::NodePtr &GetIonNode() { return i_node_; }

  private:
    //! Stores all transformation fields.
    Transform transform_;

    ion::gfx::NodePtr  i_node_;  //! Associated Ion Node.

    std::vector<ShapePtr> shapes_;
    std::vector<NodePtr>  children_;

    //! Overrides this to also set the label in the Ion node.
    virtual void SetName_(const std::string &name) override;

    //! Enables or disables the node.
    void SetEnabled_(bool enabled);

    void SetScale_(const ion::math::Vector3f &scale);
    void SetRotation_(const ion::math::Rotationf &rotation);
    void SetTranslation_(const ion::math::Vector3f &translation);

    void SetStateTable_(const ion::gfx::StateTablePtr &state_table);
    void SetShaderProgram_(const ion::gfx::ShaderProgramPtr &program);

    //! Clears the list of child nodes.
    void ClearChildren_();

    //! Adds a child.
    void AddChild_(const NodePtr &child);

    //! Adds a shape.
    void AddShape_(const ShapePtr &shape);

    friend class Reader_;
};

}  // namespace Graph
