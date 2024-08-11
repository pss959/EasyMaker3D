//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Panes/BoxPane.h"
#include "Panes/ClipPane.h"
#include "Panes/IPaneInteractor.h"
#include "Util/Memory.h"
#include "Widgets/IScrollable.h"

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
class ScrollingPane : public BoxPane, public IPaneInteractor,
                      public IScrollable {
  public:
    /// Returns the ClipPane representing the contents of the ScrollingPane.
    const ClipPanePtr & GetContentsPane() const { return contents_; }

    /// Returns the width of the scroll bar. This can be used to help with
    /// sizing Panes.
    float GetScrollBarWidth() const;

    /// Allows the scroll area size to be set.
    void SetScrollAreaSize(const Vector2f &size) { SetMinSize(size); }

    /// Defines this to also update scrolling and the thumb.
    virtual void UpdateForLayoutSize(const Vector2f &size) override;

    // IPaneInteractor interface.
    virtual IPaneInteractor * GetInteractor() override { return this; }
    virtual ClickableWidgetPtr GetActivationWidget() const override;
    virtual BorderPtr GetFocusBorder() const override;
    virtual bool HandleEvent(const Event &event) override;

    /// Scrolls to the given fraction of the way down (0 = top, 1 = bottom).
    void ScrollTo(float pos);

    /// Scrolls by the given (signed) amount.
    void ScrollBy(float amount);

    /// Scrolls so that the given Pane (which must be contained within this
    /// ScrollingPane) is visible.
    void ScrollToShowSubPane(const Pane &sub_pane);

    /// Returns the current scroll position as a fraction from 0 (top) to 1
    /// (bottom).
    float GetScrollPosition() const { return scroll_pos_; }

    // ------------------------------------------------------------------------
    // IScrollable Interface.
    // ------------------------------------------------------------------------

    /// Defines this to scroll the dropdown list if possible.
    virtual bool ProcessValuator(float delta) override;

  protected:
    ScrollingPane() {}

    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;
    virtual void CreationDone() override;

    /// Redefines this to use just the minimum size of the ScrollingPane,
    /// ignoring the ClipPane size.
    virtual Vector2f ComputeBaseSize() const override;

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

    /// Returns the height of the scrolling area (minus all padding).
    float GetContentsHeight_() const;

    friend class Parser::Registry;
};
