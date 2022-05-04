#pragma once

#include "Memory.h"
#include "Panes/IPaneInteractor.h"
#include "Panes/LeafPane.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(CheckboxPane);
DECL_SHARED_PTR(PushButtonWidget);

/// CheckboxPane is a derived LeafPane that implements an interactive checkbox.
///
/// \ingroup Panes
class CheckboxPane : public LeafPane, public IPaneInteractor {
  public:
    /// Returns the current state of the checkbox.
    bool GetState() const { return state_; }

    /// Sets the state of the checkbox.
    void SetState(bool new_state);

    virtual void PostSetUpIon() override;

    // IPaneInteractor interface.
    virtual IPaneInteractor * GetInteractor() override { return this; }
    virtual ClickableWidgetPtr GetActivationWidget() const override;
    virtual bool CanFocus() const override;
    virtual void Activate() override;

  protected:
    CheckboxPane() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<bool> state_{"state", false};
    ///@}

    PushButtonWidgetPtr button_;

    void Toggle_();
    void UpdateState_();

    friend class Parser::Registry;
};
