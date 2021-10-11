#pragma once

#include <string>
#include <vector>

#include <ion/gfx/node.h>

#include "Math/Types.h"
#include "SG/Change.h"
#include "SG/IonContext.h"
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
    /// Flags used to enable or disable specific Node behavior. Defaults are
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

    /// \name Enabling and Disabling Functions.
    ///@{

    /// Enables or disables the node behavior(s) associated with a Flag.
    void SetEnabled(Flag flag, bool b);

    /// Returns true if the given behavior is enabled.
    bool IsEnabled(Flag flag) const { return ! GetDisabledFlags().Has(flag); }

    /// Returns the set of disabled flags.
    const Util::Flags<Flag> & GetDisabledFlags() const {
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
    const Matrix4f  & GetModelMatrix() const;
    ///@}

    /// Returns the names of all shaders specified for the Node. If this is
    /// empty, the Node just uses whatever shaders it inherits. Otherwise, it
    /// overrides whichever shaders are specified (according to their pass
    /// names).
    const std::vector<std::string> & GetShaderNames() const {
        return shader_names_;
    }

    /// Returns the RenderPass name associated with the Node. This is typically
    /// empty, meaning that it should be traversed for all render passes. If it
    /// is not empty, it (and its subgraph) should not be traversed for a
    /// different render pass.
    const std::string & GetPassName() const { return pass_name_; }

    /// \name Uniform Functions.
    ///@{
    /// Returns the UniformBlock instances in the node.
    const std::vector<UniformBlockPtr> & GetUniformBlocks() const {
        return uniform_blocks_;
    }

    /// Sets the base color uniform for the node.
    void SetBaseColor(const Color &color);

    /// Sets the emissive color uniform for the node.
    void SetEmissiveColor(const Color &color);

    ///@}

    /// Returns the state table in the node.
    const StateTablePtr & GetStateTable() const { return state_table_; }

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

    /// Adds a child node at the end.
    void AddChild(const NodePtr &child);

    /// Inserts a child node at the given index.
    void InsertChild(size_t index, const NodePtr &child);

    /// Removes the child node at the given index. Asserts if the index is bad.
    void RemoveChild(size_t index);

    /// Removes the given child. Asserts if it is not found.
    void RemoveChild(const NodePtr &child);

    /// Replaces a child node at the given index. Asserts if the index is bad.
    void ReplaceChild(size_t index, const NodePtr &new_child);

    ///@}

    /// Adds a shape to the node.
    void AddShape(const ShapePtr &shape);

    /// Returns a Notifier that is invoked when a change is made to the node.
    Util::Notifier<Change> & GetChanged() { return changed_; }

    /// Returns the current Bounds in local coordinates.
    const Bounds & GetBounds() const;

    /// Convenience that returns the current Bounds scaled by the Node's scale
    /// factors.
    Bounds GetScaledBounds() const;

    /// Returns a clone of the Node.
    NodePtr CloneNode(bool is_deep) const;

    /// Returns a UniformBlock that matches the given pass name (which may be
    /// empty for pass-independent blocks). If must_exist is true, this throws
    /// an exception if it is not found. Otherwise, it just returns a null
    /// pointer.
    UniformBlockPtr GetUniformBlockForPass(const std::string &pass_name,
                                           bool must_exist) const;

    /// Sets up the Ion data in this Node. The IonContext is supplied, as is
    /// the Ion ShaderProgram to use for each RenderPass. Returns the resulting
    /// Ion Node.
    virtual ion::gfx::NodePtr SetUpIon(
        const IonContextPtr &ion_context,
        const std::vector<ion::gfx::ShaderProgramPtr> &programs);

    /// Returns the associated Ion node, which will be null until SetUpIon() is
    /// called.
    const ion::gfx::NodePtr & GetIonNode() { return ion_node_; }

    /// Enables or disables the Node, UniformBlocks, ShaderProgram, and Shapes
    /// for rendering the given pass, based on disabled flags and pass-specific
    /// behavior.
    virtual void EnableForRenderPass(const std::string &pass_name);

  protected:
    Node() {}

    /// This is called to get updated bounds for the node after something
    /// invalidates them. The Node class defines this to collect and combine
    /// bounds from all shapes and children.
    virtual Bounds UpdateBounds() const;

    /// This should be called when anything is modified in the Node; it causes
    /// all observers to be notified of the Change. Derived classes can also
    /// override this to add additional behavior.
    virtual void ProcessChange(const Change &change);

    /// This is used for setting up clones: it copies the contents from the
    /// given instance into this one. The instance is guaranteed to be of the
    /// same type. This class defines it to do nothing, as all parsed fields
    /// will already be copied or cloned correctly by the base class. Derived
    /// classes can add any extra work.
    virtual void CopyContentsFrom(const Node &from, bool is_deep) {}

  private:
    ion::gfx::NodePtr ion_node_;  /// Associated Ion Node.

    /// \name Parsed Fields
    ///@{
    Parser::FlagField<Flag>               disabled_flags_{"disabled_flags"};
    Parser::TField<std::string>           pass_name_{"pass_name"};
    Parser::TField<Vector3f>              scale_{"scale", {1, 1, 1}};
    Parser::TField<Rotationf>             rotation_{"rotation"};
    Parser::TField<Vector3f>              translation_{"translation",{0, 0, 0}};
    Parser::VField<std::string>           shader_names_{"shader_names"};
    Parser::ObjectField<StateTable>       state_table_{"state_table"};
    Parser::ObjectListField<UniformBlock> uniform_blocks_{"blocks"};
    Parser::ObjectListField<Shape>        shapes_{"shapes"};
    Parser::ObjectListField<Node>         children_{"children"};
    ///@}

    // These are all mutable because they are caches.
    mutable bool      matrices_valid_ = true;
    mutable bool      bounds_valid_   = false;
    mutable Matrix4f  matrix_         = Matrix4f::Identity();
    mutable Bounds    bounds_;

    /// Stores the IonContext used to set up the Ion data. This is needed to be
    /// stored in case a new shape or child is added to the Node that needs to
    /// be set up.
    IonContextPtr ion_context_;

    /// Stores the Ion ShaderProgram in use for each RenderPass.
    std::vector<ion::gfx::ShaderProgramPtr> programs_;

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
    void UpdateMatrices_() const;

    /// Creates, adds, and returns a UniformBlock instance for the named pass.
    UniformBlockPtr AddUniformBlock_(const std::string &pass_name);

    friend class Parser::Registry;
};

}  // namespace SG
