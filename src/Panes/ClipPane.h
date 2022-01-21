#pragma once

#include <memory>

#include "Panes/BoxPane.h"

namespace Parser { class Registry; }

/// ClipPane is a derived BoxPane that clips all contained Panes to its
/// rectangle using stenciling.
class ClipPane : public BoxPane {
  public:
    /// Returns the size of the contents if no clipping were applied.
    Vector2f GetUnclippedSize() const;

    /// Moves the clipped contents to the given offset.
    void SetContentsOffset(const Vector2f &offset);

    /// Returns the current offset of the clipped contents.
    Vector2f GetContentsOffset() const;

  protected:
    ClipPane() {}

    virtual void CreationDone() override;

    /// Redefines this to return the node that clips the contained panes.
    virtual SG::Node & GetExtraChildParent() { return GetContentsNode_(); }

    /// Redefines this to just use the base size of the ClipPane, not the base
    /// size of the contents of the clip area.
    virtual Vector2f ComputeBaseSize() const override;

    /// Redefines this to just use the bounds of the clip rectangle.
    virtual Bounds UpdateBounds() const override;

    /// Redefines this to pass along a size change only when the clip rectangle
    /// changes size.
    virtual void SizeChanged(const Pane &initiating_pane) override;

  private:
    /// Node contained panes are added to (as extra children).
    mutable SG::NodePtr pane_parent_;

    /// Node that is scaled to the clip_size.
    SG::NodePtr clip_node_;

    /// Returns the Node representing the clipped contents.
    SG::Node & GetContentsNode_() const;

    friend class Parser::Registry;
};

typedef std::shared_ptr<ClipPane> ClipPanePtr;
