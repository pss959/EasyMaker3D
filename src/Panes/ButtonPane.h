//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Panes/BoxPane.h"
#include "Panes/IPaneInteractor.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ButtonPane);
DECL_SHARED_PTR(PushButtonWidget);

/// ButtonPane is a derived BoxPane that treats all contained Panes as an
/// interactive push button.
///
/// \ingroup Panes
class ButtonPane : public BoxPane, public IPaneInteractor {
  public:
    /// ButtonPane has to be named since that is what is reported to observers.
    virtual bool IsNameRequired() const override { return true; }

    /// Returns the PushButtonWidget for the ButtonPane.
    PushButtonWidget & GetButton() const;

    /// Enables or disables the button for interacting and changes the color to
    /// indicate whether the button is enabled.
    void SetInteractionEnabled(bool enabled);

    /// Returns true if the button has interaction enabled.
    bool IsInteractionEnabled() const;

    /// Sets a flag indicating whether the ButtonPane should be focused when
    /// activated. The default is true.
    void SetShouldFocusOnActivation(bool b) { should_focus_on_activation_ = b; }

    /// Returns a flag indicating whether the ButtonPane should be focused when
    /// activated. The default is true.
    bool ShouldFocusOnActivation() const { return should_focus_on_activation_; }

    // IPaneInteractor interface.
    virtual IPaneInteractor * GetInteractor() override { return this; }
    virtual ClickableWidgetPtr GetActivationWidget() const override;
    virtual bool CanFocus(FocusReason reason) const override;
    virtual BorderPtr GetFocusBorder() const override;

  protected:
    ButtonPane() {}

    virtual void AddFields() override;
    virtual void CreationDone() override;

    /// Redefines this to return the PushButtonWidget so that borders and
    /// background are part of the button.
    virtual SG::Node & GetAuxParent() override;

    /// Redefines this to clear out the children of the PushButtonWidget that
    /// were copied from Panes.
    virtual void CopyContentsFrom(const Parser::Object &from,
                                  bool is_deep) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<bool> is_toggle_;
    ///@}

    bool should_focus_on_activation_ = true;

    mutable PushButtonWidgetPtr button_;

    friend class Parser::Registry;
};
