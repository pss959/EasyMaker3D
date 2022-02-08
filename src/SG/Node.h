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

        /// Disables searching for this node and its subgraph.
        kSearch       = (1 << 4),
    };

    virtual ~Node();

    /// Nodes are scoped.
    virtual bool IsScoped() const override { return true; }

    /// \name Enabling and Disabling Functions.
    ///@{

    /// Enables or disables the node behavior(s) associated with a Flag.
    void SetFlagEnabled(Flag flag, bool b);

    /// Returns true if the given behavior is enabled.
    bool IsFlagEnabled(Flag flag) const {
        return ! GetDisabledFlags().Has(flag);
    }

    /// Shorthand for setting the Flag::kTraversal flag.
    void SetEnabled(bool b) { SetFlagEnabled(Flag::kTraversal, b); }

    /// Shorthand for testing the Flag::kTraversal flag.
    bool IsEnabled() const { return IsFlagEnabled(Flag::kTraversal); }

    /// Returns the set of disabled flags.
    const Util::Flags<Flag> & GetDisabledFlags() const {
        return disabled_flags_;
    }

    ///@}

    /// \name Transformation Modification Functions.
    /// Each of these updates the Node and its Ion Matrix uniform.
    ///@{
    void SetUniformScale(float s) { SetScale(Vector3f(s, s, s)); }
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

    /// Returns a flag indicating that the Node bounds should be used for
    /// intersection testing rather than testing shapes and children. This can
    /// be useful to allow picking of sparse or complex objects when exact
    /// intersections are not needed, such as for 3D icons.
    bool ShouldUseBoundsProxy() const { return use_bounds_proxy_; }

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
    /// Each of these operates on the contents of the "children" field. To get
    /// all Nodes that are considered children, call GetAllChildren().
    ///@{

    /// Returns the child nodes in the children field of the node.
    const std::vector<NodePtr> & GetChildren() const { return children_; }

    /// Returns the number of child nodes in the children field.
    size_t GetChildCount() const { return GetChildren().size(); }

    /// Returns the index of the given child node, or -1 if it is not found.
    int GetChildIndex(const NodePtr &child) const;

    /// Returns the indexed child node, or null if the index is bad.
    NodePtr GetChild(size_t index) const;

    /// Returns all Nodes that are considered children of this Node. This may
    /// include Nodes added as extra children from other fields in derived
    /// classes.
    std::vector<NodePtr> GetAllChildren() const;

    ///@}

    /// \name Extra Child Functions.
    /// Derived classes may contain other fields besides "children" that need
    /// to treat their contents as children. These functions allow them to do
    /// so. Extra children are included in the vector returned by
    /// GetAllChildren().
    ///@{

    /// Clears the list of extra children.
    void ClearExtraChildren();

    /// Adds the given Node as an extra child, setting it up properly.
    void AddExtraChild(const NodePtr &child);

    /// Returns the current list of extra children.
    const std::vector<NodePtr> & GetExtraChildren() const {
        return extra_children_;
    }

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

    /// Removes all children.
    void ClearChildren();

    ///@}

    /// Adds a shape to the node.
    void AddShape(const ShapePtr &shape);

    /// Returns the current Bounds in object coordinates.
    const Bounds & GetBounds() const;

    /// Convenience that returns the current Bounds scaled by the Node's scale
    /// factors.
    Bounds GetScaledBounds() const;

    /// Returns a UniformBlock that matches the given pass name (which may be
    /// empty for pass-independent blocks). This creates the block if it is not
    /// found.
    UniformBlock & GetUniformBlockForPass(const std::string &pass_name);

    /// Sets up the Ion data in this Node. The IonContext is supplied, as is
    /// the Ion ShaderProgram to use for each RenderPass. Returns the resulting
    /// Ion Node. Derived classes can redefine this to add extra work, but
    /// should call the base class first.
    virtual ion::gfx::NodePtr SetUpIon(
        const IonContextPtr &ion_context,
        const std::vector<ion::gfx::ShaderProgramPtr> &programs);

    /// Returns the associated Ion node, which will be null until SetUpIon() is
    /// called.
    const ion::gfx::NodePtr & GetIonNode() const { return ion_node_; }

    /// This makes it easier for derived classes to add code to execute just
    /// after SetUpIon() is called. The base class defines it to do nothing.
    virtual void PostSetUpIon() {}

    /// Updates the Node for the given RenderPass. The base class defines this
    /// to enable or disable the Node, UniformBlocks, ShaderProgram, and Shapes
    /// for rendering the given pass, based on disabled flags and pass-specific
    /// behavior. Derived classes can add other update code.
    virtual void UpdateForRenderPass(const std::string &pass_name);

  protected:
    /// This can be used to make changes to the given Node without notifying
    /// observers. Notification is disabled as long as the instance is in
    /// scope.
    class NotificationDisabler {
      public:
        NotificationDisabler(Node &node) :
            node_(node), was_enabled_(node.IsNotifyEnabled()) {
            node_.SetNotifyEnabled(false);
        }
        ~NotificationDisabler() {
            if (was_enabled_)
                node_.SetNotifyEnabled(true);
        }
      private:
        Node       &node_;
        const bool was_enabled_;
    };

    Node() {}

    virtual void AddFields() override;

    /// Redefines this to set up this Node as an observer for all children and
    /// shapes defined in fields.
    virtual void CreationDone() override;

    /// Sets the flag indicating that the Node bounds should be used for
    /// intersection testing rather than testing shapes and children.
    void SetUseBoundsProxy(bool use_proxy) { use_bounds_proxy_ = use_proxy; }

    /// This is called to get updated bounds for the node after something
    /// invalidates them. The Node class defines this to collect and combine
    /// bounds from all shapes and children.
    virtual Bounds UpdateBounds() const;

    /// Redefines this to invalidate bounds and matrices if necessary.
    virtual bool ProcessChange(Change change, const Object &obj) override;

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
    Parser::TField<bool>                  use_bounds_proxy_{"use_bounds_proxy",
                                                            false};
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

    /// Storage for extra children added with AddExtraChild().
    std::vector<NodePtr> extra_children_;

    /// Ion Shapes cannot be enabled or disabled. To disable rendering shapes,
    /// they are temporarily moved into this vector.
    std::vector<ion::gfx::ShapePtr> saved_shapes_;

    /// Sets up a child Node that has been added. This adds the Ion child (if
    /// the Ion node has been set up) and adds this as an observer.
    void SetUpChild_(Node &child);

    /// Does the opposite of SetUpChild_() for a child Node being removed.
    void UnsetUpChild_(Node &child);

    /// Enables or disables Ion shape rendering by moving them into
    /// saved_shapes_ or back.
    void EnableShapes_(bool enabled);

    /// Updates the matrix_ field and the Ion matrix uniforms when a transform
    /// field changes.
    void UpdateMatrices_() const;

    /// Returns an existing UniformBlock for the named pass or null.
    UniformBlockPtr FindUniformBlockForPass_(
        const std::string &pass_name) const;

    /// Creates, adds, and returns a UniformBlock instance for the named pass.
    UniformBlockPtr AddUniformBlock_(const std::string &pass_name);

    friend class Parser::Registry;
};

}  // namespace SG
