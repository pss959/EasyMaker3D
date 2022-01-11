#pragma once

#include <memory>

#include "Panes/ClipPane.h"

namespace Parser { class Registry; }

/// ScrollingPane is a derived BoxPane that allows for scrolling of its
/// contents within a fixed area. Note that the area's size can change in
/// response to size requests from above, but not in response to changes to the
/// contents. The contents field must be a ClipPane. No other child panes are
/// allowed to be added.
class ScrollingPane : public BoxPane {
  public:
    /// Defines this to also update scrolling.
    virtual void SetSize(const Vector2f &size) override;

    virtual bool IsInteractive()        const override { return true; }
    virtual bool IsInteractionEnabled() const override { return true; }
    virtual bool HandleEvent(const Event &event) override;

    /// Scrolls to the top of the contents.
    void ScrollToTop();

    /// Scrolls by the given (signed) amount.
    void ScrollBy(float amount);

  protected:
    ScrollingPane() {}

  private:
    /// Current position of the scrolled area. 0 is at the top, 1 is at the
    /// bottom.
    float scroll_pos_ = 0;

    /// Updates the scroll translation based on scroll_pos_.
    void UpdateScroll_();

    friend class Parser::Registry;
};

typedef std::shared_ptr<ScrollingPane> ScrollingPanePtr;
