#pragma once

#include <memory>

#include "Panes/BoxPane.h"

namespace Parser { class Registry; }

/// ClipPane is a derived BoxPane that clips all contained Panes to its
/// rectangle using stenciling.
class ClipPane : public BoxPane {
  public:
    virtual void AllFieldsParsed(bool is_template) override;

    /// Returns the size of the contents if no clipping were applied.
    Vector2f GetUnclippedSize() const;

    /// Moves the clipped contents to the given offset.
    void SetContentsOffset(const Vector2f &offset);

    /// Returns the current offset of the clipped contents.
    Vector2f GetContentsOffset() const;

  protected:
    ClipPane() {}

    /// Redefines this to return the node that clips the contained panes.
    virtual SG::Node & GetExtraChildParent() { return *pane_parent_; }

    /// Redefines this to not use the minimum size of the contents of the clip
    /// area.
    virtual Vector2f ComputeMinSize() const override;

    /// Redefines this to just use the bounds of the clip rectangle.
    virtual Bounds UpdateBounds() const override;

    /// Redefines this to pass along a size change only when the clip rectangle
    /// changes size.
    virtual void ProcessSizeChange(const Pane &initiating_pane) override;

  private:
    /// Node contained panes are added to (as extra children).
    SG::NodePtr pane_parent_;

    /// Node that is scaled to the clip_size.
    SG::NodePtr clip_node_;

    friend class Parser::Registry;
};

typedef std::shared_ptr<ClipPane> ClipPanePtr;
