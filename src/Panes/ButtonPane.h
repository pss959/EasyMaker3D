#pragma once

#include "Memory.h"
#include "Panes/BoxPane.h"
#include "Panes/IPaneInteractor.h"

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

    // IPaneInteractor interface.
    virtual IPaneInteractor * GetInteractor() override { return this; }
    virtual ClickableWidgetPtr GetActivationWidget() const override;
    virtual bool CanFocus() const override;
    virtual void Activate() override;

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
    Parser::TField<bool> is_toggle_{"is_toggle", false};
    ///@}

    mutable PushButtonWidgetPtr button_;

    friend class Parser::Registry;
};
