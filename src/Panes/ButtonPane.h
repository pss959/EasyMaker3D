#pragma once

#include <memory>

#include "Panes/BoxPane.h"
#include "Widgets/PushButtonWidget.h"

namespace Parser { class Registry; }

/// ButtonPane is a derived BoxPane that treats all contained Panes as a push
/// button.
class ButtonPane : public BoxPane {
  public:
    /// ButtonPane has to be named since that is what is reported to observers.
    virtual bool IsNameRequired() const override { return true; }

    /// Returns the PushButtonWidget for the ButtonPane.
    PushButtonWidget & GetButton() const;

    /// Enables or disables the button for interacting and changes the color to
    /// indicate whether the button is enabled.
    void SetInteractionEnabled(bool enabled);

    virtual bool IsInteractive() const override { return true; }
    virtual bool IsInteractionEnabled() const;
    virtual void Activate() override;

  protected:
    ButtonPane() {}

    /// Redefines this to return the PushButtonWidget so that border,
    /// background, and all contained Panes are part of the button.
    virtual SG::Node & GetAuxParent() override { return GetButton(); }

  private:
    mutable PushButtonWidgetPtr button_;

    friend class Parser::Registry;
};

typedef std::shared_ptr<ButtonPane> ButtonPanePtr;
