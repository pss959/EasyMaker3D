#pragma once

#include "Base/Memory.h"
#include "Panes/BoxPane.h"
#include "Panes/ClipPane.h"
#include "Panes/IPaneInteractor.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ScrollingPane);
DECL_SHARED_PTR(SliderPane);

/// ScrollingPane is a derived BoxPane that allows for scrolling of its
/// contents within a fixed area. Note that the area's size can change in
/// response to size requests from above, but not in response to changes to the
/// contents. The contents field must be a ClipPane. No other child panes are
/// allowed to be added.
///
/// \ingroup Panes
class ScrollingPane : public BoxPane, public IPaneInteractor {
  public:
    /// Returns the ClipPane representing the contents of the ScrollingPane.
    const ClipPanePtr & GetContentsPane() const { return contents_; }

    /// Returns the width of the scroll bar. This can be used to help with
    /// sizing Panes.
    float GetScrollBarWidth() const;

    /// Allows the scroll area size to be set.
    void SetScrollAreaSize(const Vector2f &size) { SetMinSize(size); }

    /// Defines this to also update scrolling and the thumb.
    virtual void SetLayoutSize(const Vector2f &size) override;

    // IPaneInteractor interface.
    virtual IPaneInteractor * GetInteractor() override { return this; }
    virtual ClickableWidgetPtr GetActivationWidget() const override;
    virtual bool CanFocus() const override;
    virtual bool HandleEvent(const Event &event) override;

    /// Scrolls to the given fraction of the way down (0 = top, 1 = bottom).
    void ScrollTo(float pos);

    /// Scrolls by the given (signed) amount.
    void ScrollBy(float amount);

  protected:
    ScrollingPane() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::ObjectField<ClipPane> contents_;
    ///@}

    SliderPanePtr slider_pane_;
    SG::NodePtr   thumb_;

    /// Current position of the scrolled area. 0 is at the top, 1 is at the
    /// bottom.
    float scroll_pos_ = 0;

    /// Amount to scroll per change in scroll_pos_.
    float scroll_factor_ = 1;

    /// Updates the scroll translation based on scroll_pos_.
    void UpdateScroll_();

    friend class Parser::Registry;
};
