//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Panes/IPaneInteractor.h"
#include "Panes/LeafPane.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(CheckboxPane);
DECL_SHARED_PTR(PushButtonWidget);

/// CheckboxPane is a derived LeafPane that implements an interactive checkbox.
///
/// \ingroup Panes
class CheckboxPane : public LeafPane, public IPaneInteractor {
  public:
    /// Returns a Notifier that is invoked when the button changes state.
    Util::Notifier<> & GetStateChanged() { return state_changed_; }

    /// Returns the current state of the checkbox.
    bool GetState() const { return state_; }

    /// Sets the state of the checkbox.
    void SetState(bool new_state);

    // IPaneInteractor interface.
    virtual IPaneInteractor * GetInteractor() override { return this; }
    virtual ClickableWidgetPtr GetActivationWidget() const override;
    virtual BorderPtr GetFocusBorder() const override;

  protected:
    CheckboxPane() {}

    virtual void CreationDone() override;
    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<bool> state_;
    ///@}

    /// Notifies when the button state changes.
    Util::Notifier<> state_changed_;

    PushButtonWidgetPtr button_;

    void Toggle_();
    void UpdateState_();

    friend class Parser::Registry;
};
