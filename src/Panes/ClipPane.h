#pragma once

#include <memory>

#include "Panes/BoxPane.h"

namespace Parser { class Registry; }

/// ClipPane is a derived BoxPane that clips all contained Panes to a rectangle
/// specified as a field using stenciling.
class ClipPane : public BoxPane {
  public:
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void AllFieldsParsed(bool is_template) override;

    void SetClipSize(const Vector2f &size);
    void SetClipOffset(const Vector2f &offset);
    const Vector2f & GetClipSize()   const { return clip_size_; }
    const Vector2f & GetClipOffset() const { return clip_offset_; }

  protected:
    ClipPane() {}

    /// Redefines this to return the node that clips the contained panes.
    virtual SG::Node & GetExtraChildParent() { return *pane_parent_; }

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Vector2f> clip_offset_{"clip_offset", {0,0}};
    Parser::TField<Vector2f> clip_size_{"clip_size", {1,1}};
    ///@}

    /// Node contained panes are added to (as extra children).
    SG::NodePtr pane_parent_;

    /// Node that is scaled to the clip_size.
    SG::NodePtr clip_node_;

    friend class Parser::Registry;
};

typedef std::shared_ptr<ClipPane> ClipPanePtr;
