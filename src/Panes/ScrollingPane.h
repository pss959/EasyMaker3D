#pragma once

#include <memory>

#include "Panes/BoxPane.h"

namespace Parser { class Registry; }

/// ScrollingPane is a derived BoxPane that allows for XXXX
class ScrollingPane : public BoxPane {
  public:
    virtual void AddFields() override;

    /// Replaces the current contained Panes with the given ones.
    void SetPanes(const std::vector<PanePtr> &new_panes);

    virtual bool IsInteractive()        const override { return true; }
    virtual bool IsInteractionEnabled() const override { return true; }
    virtual bool HandleEvent(const Event &event) override;

  protected:
    ScrollingPane() {}

    /// Redefines this to not use the minimum size of the contents of the
    /// scrolling area.
    virtual Vector2f ComputeMinSize() const override;

  private:
    /// \name Parsed Fields
    ///@{
    // XXXX
    ///@}

    friend class Parser::Registry;
};

typedef std::shared_ptr<ScrollingPane> ScrollingPanePtr;
