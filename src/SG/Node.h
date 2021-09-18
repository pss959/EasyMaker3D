#pragma once

#include <string>
#include <vector>

#include <ion/gfx/node.h>

#include "Math/Types.h"
#include "SG/Change.h"
#include "SG/Object.h"
#include "SG/Shape.h"
#include "SG/StateTable.h"
#include "SG/Typedefs.h"
#include "SG/UniformBlock.h"
#include "Util/Enum.h"
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

    //! Redefines this to set up notification and detect transform changes.
    virtual void AllFieldsParsed() override;

    //! Returns the associated Ion node, which is null until CreateIonNode() is
    //! called.
    const ion::gfx::NodePtr & GetIonNode() { return ion_node_; }

    void CreateIonNode();

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

    //! Sets the base color uniform for the node.
    void SetBaseColor(const Color &color);

    //! Sets the emissive color uniform for the node.
    void SetEmissiveColor(const Color &color);

    //! Returns the state table in the node.
    const StateTablePtr & GetStateTable() const { return state_table_; }

    //! Returns the UniformBlock instances in the node.
    const std::vector<UniformBlockPtr> & GetUniformBlocks() const {
        return uniform_blocks_;
    }
    //! Returns the shapes in the node.
    const std::vector<ShapePtr>    & GetShapes()   const { return shapes_;   }
    //! Returns the child nodes in the node.
    const std::vector<NodePtr>     & GetChildren() const { return children_; }

    //! Returns a Notifier that is invoked when a change is made to the shape.
    Util::Notifier<Change> & GetChanged() { return changed_; }

    //! Returns the current Bounds in local coordinates.
    const Bounds & GetBounds();

    //! Updates the Node for rendering.
    virtual void UpdateForRendering();

  protected:
    //! This is called to get updated bounds for the node after something
    //! invalidates them. The Node class defines this to collect and combine
    //! bounds from all shapes and children.
    virtual Bounds UpdateBounds();

    //! Returns a UniformBlock that matches the given pass name (which may be
    //! empty for pass-independent blocks). If must_exist is true, this throws
    //! an exception if it is not found. Otherwise, it just returns a null
    //! pointer.
    UniformBlockPtr GetUniformBlockForPass(const std::string &pass_name,
                                           bool must_exist);

    //! Creates, adds, and returns a UniformBlock instance for the named pass.
    UniformBlockPtr AddUniformBlock(const std::string &pass_name);

    //! This should be called when anything is modified in the Node; it causes
    //! all observers to be notified of the Change. Derived classes can also
    //! override this to add additional behavior.
    virtual void ProcessChange(const Change &change);

    //! Lets derived classes add shapes to the node.
    void AddShape(const ShapePtr &shape);

  private:
    ion::gfx::NodePtr ion_node_;  //! Associated Ion Node.

    //! \name Parsed Fields
    //!@{
    Parser::FlagField<Flag>               disabled_flags_{"disabled_flags"};
    Parser::TField<Vector3f>              scale_{"scale", {1, 1, 1}};
    Parser::TField<Rotationf>             rotation_{"rotation"};
    Parser::TField<Vector3f>              translation_{"translation",{0, 0, 0}};
    Parser::ObjectField<StateTable>       state_table_{"state_table"};
    Parser::ObjectListField<UniformBlock> uniform_blocks_{"blocks"};
    Parser::ObjectListField<Shape>        shapes_{"shapes"};
    Parser::ObjectListField<Node>         children_{"children"};
    //!@}

    bool      matrices_valid_ = true;  // Assume true until transform changes.
    bool      bounds_valid_   = false;
    Matrix4f  matrix_         = Matrix4f::Identity();
    Bounds    bounds_;

    //! Notifies when a change is made to the node or its subgraph.
    Util::Notifier<Change> changed_;

    //! Adds this Node as an observer of the given Shape.
    void AddAsShapeObserver_(Shape &shape);

    //! Adds this Node as an observer of the given child Node.
    void AddAsChildNodeObserver_(Node &child);

    //! Updates the matrix_ field and the Ion matrix uniforms when a transform
    //! field changes.
    void UpdateMatrices_();
};

}  // namespace SG
