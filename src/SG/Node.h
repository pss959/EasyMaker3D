#pragma once

#include <string>
#include <vector>

#include <ion/gfx/node.h>

#include "Math/Types.h"
#include "SG/Change.h"
#include "SG/Interactor.h"
#include "SG/Object.h"
#include "SG/ShaderProgram.h"
#include "SG/Shape.h"
#include "SG/StateTable.h"
#include "SG/Typedefs.h"
#include "SG/UniformBlock.h"
#include "Util/Flags.h"
#include "Util/Notifier.h"

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
    };

    //! Default constructor.
    Node();

    //! Constructor that sets the name, primarily for testing.
    Node(const std::string &name);

    virtual void AddFields() override;

    //! Returns the associated Ion node.
    const ion::gfx::NodePtr & GetIonNode() { return ion_node_; }

    //! Enables or disables the node behavior(s) associated with a DisableFlag.
    void SetEnabled(Flag flag, bool b) {
        // Inverse setting, since flags indicate what is disabled.
        if (b)
            GetDisabledFlags().Reset(flag);
        else
            GetDisabledFlags().Set(flag);
    }

    //! Returns true if the given behavior is enabled.
    bool IsEnabled(Flag flag) const {
        return ! GetDisabledFlags().Has(flag);
    }

    //! Returns the set of disabled flags.
    const Util::Flags<Flag> & GetDisabledFlags() const {
        return disabled_flags_;
    }

    //! Returns the set of disabled flags.
    Util::Flags<Flag> & GetDisabledFlags() {
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
    const Matrix4f  & GetModelMatrix();
    //!@}

    //! Returns the state table in the node.
    const StateTablePtr & GetStateTable() const { return state_table_; }
    //! Returns the shader program in the node.
    const ShaderProgramPtr & GetShaderProgram() const {
        return shader_program_;
    }

    //! Returns the uniform blocks in the node.
    const std::vector<UniformBlockPtr> & GetUniformBlocks() const {
        return uniform_blocks_;
    }
    //! Returns the shapes in the node.
    const std::vector<ShapePtr>   & GetShapes()   const { return shapes_;   }
    //! Returns the child nodes in the node.
    const std::vector<NodePtr>    & GetChildren() const { return children_; }
    //! Returns the Interactors attached to the node.
    const std::vector<InteractorPtr> & GetInteractors() const {
        return interactors_;
    }

    //! Returns a UniformBlock that matches the given pass name. An empty name
    //! is valid. Returns a null pointer if it is not found.
    UniformBlockPtr GetUniformBlockForPass(const std::string &pass_name);

    //! Returns a Notifier that is invoked when a change is made to the shape.
    Util::Notifier<Change> & GetChanged() { return changed_; }

    //! Returns the current Bounds in local coordinates.
    const Bounds & GetBounds();

    //! Updates all state in the Node if necessary.
    void Update();

    //! Updates for rendering the render pass with the given name. This enables
    //! or disables UniformBlock instances that are pass-specific.
    void UpdateForRenderPass(const std::string &pass_name);

    virtual void SetUpIon(IonContext &context) override;

  protected:
    //! Allows derived classes to set the Ion Node.
    void SetIonNode(const ion::gfx::NodePtr &node) { ion_node_ = node; }

  private:
    ion::gfx::NodePtr ion_node_;  //! Associated Ion Node.

    //! \name Parsed Fields
    //!@{
    Parser::FlagField<Flag>               disabled_flags_{"disabled_flags"};
    Parser::TField<Vector3f>              scale_{"scale", {1, 1, 1}};
    Parser::TField<Rotationf>             rotation_{"rotation"};
    Parser::TField<Vector3f>              translation_{"translation",
                                                       {0, 0, 0}};
    Parser::ObjectField<StateTable>       state_table_{"state_table"};
    Parser::ObjectField<ShaderProgram>    shader_program_{"shader"};
    Parser::ObjectListField<UniformBlock> uniform_blocks_{"uniforms"};
    Parser::ObjectListField<Shape>        shapes_{"shapes"};
    Parser::ObjectListField<Node>         children_{"children"};
    Parser::ObjectListField<Interactor>   interactors_{"interactors"};
    //!@}

    bool      matrices_valid_ = true;  // Assume true until transform changes.
    bool      bounds_valid_   = false;
    Matrix4f  matrix_         = Matrix4f::Identity();
    Bounds    bounds_;

    //! Notifies when a change is made to the node or its subgraph.
    Util::Notifier<Change> changed_;

    //! This is called when anything is modified in the Node; it causes all
    //! observers to be notified of the Change.
    void ProcessChange_(const Change &change);

    //! Updates the matrix_ field and the Ion matrix uniforms when a transform
    //! field changes.
    void UpdateMatrices_();

    //! Updates the Bounds when someing invalidates them.
    void UpdateBounds_();
};

}  // namespace SG
