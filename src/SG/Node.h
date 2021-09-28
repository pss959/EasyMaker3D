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

namespace Parser { class Registry; }

namespace SG {

/// The Node class represents the main type of object constructing a scene
/// graph.  It contains an Ion Node.
class Node : public Object {
  public:
    /// Flags used to enabled or disable specific Node behavior. Defaults are
    /// all false, meaning that nothing is disabled.
    enum class Flag : uint32_t {
        /// Disables all traversals. If a node has this flag set, rendering and
        /// intersection traversals skip this node and the subgraph below it.
        kTraversal    = (1 << 0),

        /// Disables rendering for just this node. The node's uniforms and
        /// shapes are ignored, but traversal continues to its children.
        kRender       = (1 << 1),

        /// Disables intersection testing for just this node. The node's
        /// uniforms and shapes are ignored, but traversal continues to its
        /// children.
        kIntersect    = (1 << 2),

        /// Disables intersection testing for this node and its subgraph.
        kIntersectAll = (1 << 3),
    };

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

    /// Convenience that creates and returns a Node with the given name
    static NodePtr Create(const std::string &name);

    /// Returns the associated Ion node, which is null until CreateIonNode() is
    /// called.
    const ion::gfx::NodePtr & GetIonNode() { return ion_node_; }

    void CreateIonNode();

    /// \name Enabling and Disabling Functions.
    ///@{

    /// Enables or disables the node behavior(s) associated with a DisableFlag.
    void SetEnabled(Flag flag, bool b) {
        // Inverse setting, since flags indicate what is disabled.
        if (b)
            GetDisabledFlags().Reset(flag);
        else
            GetDisabledFlags().Set(flag);
    }

    /// Returns true if the given behavior is enabled.
    bool IsEnabled(Flag flag) const {
        return ! GetDisabledFlags().Has(flag);
    }

    /// Returns the set of disabled flags.
    const Util::Flags<Flag> & GetDisabledFlags() const {
        return disabled_flags_;
    }

    /// Returns the set of disabled flags.
    Util::Flags<Flag> & GetDisabledFlags() {
        return disabled_flags_;
    }

    ///@}

    /// \name Transformation Modification Functions.
    /// Each of these updates the Node and its Ion Matrix uniform.
    ///@{
    void SetScale(const Vector3f &scale);
    void SetRotation(const Rotationf &rotation);
    void SetTranslation(const Vector3f &translation);
    ///@}

    /// \name Transformation Query Functions.
    ///@{
    const Vector3f  & GetScale()       const { return scale_;       }
    const Rotationf & GetRotation()    const { return rotation_;    }
    const Vector3f  & GetTranslation() const { return translation_; }
    const Matrix4f  & GetModelMatrix();
    ///@}

    /// Sets the base color uniform for the node.
    void SetBaseColor(const Color &color);

    /// Sets the emissive color uniform for the node.
    void SetEmissiveColor(const Color &color);

    /// Returns the state table in the node.
    const StateTablePtr & GetStateTable() const { return state_table_; }

    /// Returns the UniformBlock instances in the node.
    const std::vector<UniformBlockPtr> & GetUniformBlocks() const {
        return uniform_blocks_;
    }
    /// Returns the shapes in the node.
    const std::vector<ShapePtr>    & GetShapes()   const { return shapes_;   }

    /// \name Child Query Functions.
    ///@{

    /// Returns the child nodes in the node.
    const std::vector<NodePtr> & GetChildren() const { return children_; }

    /// Returns the number of child nodes.
    size_t GetChildCount() const { return GetChildren().size(); }

    /// Returns the index of the given child node, or -1 if it is not found.
    int GetChildIndex(const NodePtr &child) const;

    /// Returns the indexed child node, or null if the index is bad.
    NodePtr GetChild(size_t index) const;

    ///@}

    /// \name Child Modification Functions.
    /// Each of these makes sure the field containing the children is updated
    /// properly and is marked as being modified.
    ///@{

    /// Lets derived classes add a child node.
    void AddChild(const NodePtr &child);

    /// Lets derived classes insert a child node at the given index.
    void InsertChild(size_t index, const NodePtr &child);

    /// Lets derived classes remove the child node at the given index. Asserts
    /// if the index is bad.
    void RemoveChild(size_t index);

    /// Lets derived classes replace a child node at the given index. Asserts
    /// if the index is bad.
    void ReplaceChild(size_t index, const NodePtr &new_child);

    ///@}

    /// Returns a Notifier that is invoked when a change is made to the shape.
    Util::Notifier<Change> & GetChanged() { return changed_; }

    /// Returns the current Bounds in local coordinates.
    const Bounds & GetBounds();

    /// Convenience that returns the current Bounds scaled by the Node's scale
    /// factors.
    Bounds GetScaledBounds();

    /// Updates the Node for rendering.
    virtual void UpdateForRendering();

  protected:
    Node() {}

    /// This is called to get updated bounds for the node after something
    /// invalidates them. The Node class defines this to collect and combine
    /// bounds from all shapes and children.
    virtual Bounds UpdateBounds();

    /// Returns a UniformBlock that matches the given pass name (which may be
    /// empty for pass-independent blocks). If must_exist is true, this throws
    /// an exception if it is not found. Otherwise, it just returns a null
    /// pointer.
    UniformBlockPtr GetUniformBlockForPass(const std::string &pass_name,
                                           bool must_exist);

    /// Creates, adds, and returns a UniformBlock instance for the named pass.
    UniformBlockPtr AddUniformBlock(const std::string &pass_name);

    /// This should be called when anything is modified in the Node; it causes
    /// all observers to be notified of the Change. Derived classes can also
    /// override this to add additional behavior.
    virtual void ProcessChange(const Change &change);

    /// Lets derived classes add shapes to the node.
    void AddShape(const ShapePtr &shape);

  private:
    ion::gfx::NodePtr ion_node_;  /// Associated Ion Node.

    /// \name Parsed Fields
    ///@{
    Parser::FlagField<Flag>               disabled_flags_{"disabled_flags"};
    Parser::TField<Vector3f>              scale_{"scale", {1, 1, 1}};
    Parser::TField<Rotationf>             rotation_{"rotation"};
    Parser::TField<Vector3f>              translation_{"translation",{0, 0, 0}};
    Parser::ObjectField<StateTable>       state_table_{"state_table"};
    Parser::ObjectListField<UniformBlock> uniform_blocks_{"blocks"};
    Parser::ObjectListField<Shape>        shapes_{"shapes"};
    Parser::ObjectListField<Node>         children_{"children"};
    ///@}

    bool      matrices_valid_ = true;  // Assume true until transform changes.
    bool      bounds_valid_   = false;
    Matrix4f  matrix_         = Matrix4f::Identity();
    Bounds    bounds_;

    /// Ion Shapes cannot be enabled or disabled. To disable rendering shapes,
    /// they are temporarily moved into this vector.
    std::vector<ion::gfx::ShapePtr> saved_shapes_;

    /// Notifies when a change is made to the node or its subgraph.
    Util::Notifier<Change> changed_;

    /// Enables or disables Ion shape rendering by moving them into
    /// saved_shapes_ or back.
    void EnableShapes_(bool enabled);

    /// Adds this Node as an observer of the given Shape.
    void AddAsShapeObserver_(Shape &shape);

    /// Adds this Node as an observer of the given child Node.
    void AddAsChildNodeObserver_(Node &child);

    /// Removes this Node as an observer of the given child Node.
    void RemoveAsChildNodeObserver_(Node &child);

    /// Updates the matrix_ field and the Ion matrix uniforms when a transform
    /// field changes.
    void UpdateMatrices_();

    friend class Parser::Registry;
};

}  // namespace SG
