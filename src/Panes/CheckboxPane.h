#pragma once

#include "Memory.h"
#include "Panes/LeafPane.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(CheckboxPane);

/// CheckboxPane is a derived LeafPane that implements an interactive checkbox.
class CheckboxPane : public LeafPane {
  public:
    /// Returns the current state of the checkbox.
    bool GetState() const { return state_; }

    /// Sets the state of the checkbox.
    void SetState(bool new_state);

    virtual void PostSetUpIon() override;

    virtual bool IsInteractive()        const override { return true; }
    virtual bool IsInteractionEnabled() const override { return true; }
    virtual void Activate()   override;
    virtual void Deactivate() override;
    virtual bool HandleEvent(const Event &event) override;

  protected:
    CheckboxPane() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<bool> state_{"state", false};
    ///@}

    void Toggle_();
    void UpdateState_();

    friend class Parser::Registry;
};
