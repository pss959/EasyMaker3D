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
    PushButtonWidget & GetButton();

    virtual bool IsInteractive() const override { return true; }

  protected:
    ButtonPane() {}

    /// Redefines this to return the PushButtonWidget so that border,
    /// background, and all contained Panes are part of the button.
    virtual SG::Node & GetAuxParent() override { return GetButton(); }

  private:
    PushButtonWidgetPtr button_;

    friend class Parser::Registry;
};

typedef std::shared_ptr<ButtonPane> ButtonPanePtr;
