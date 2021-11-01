#pragma once

#include <memory>

#include "Panes/BoxPane.h"
#include "Widgets/PushButtonWidget.h"

namespace Parser { class Registry; }

/// ButtonPane is a derived BoxPane that treats all contained Panes as a push
/// button.
class ButtonPane : public BoxPane {
  public:
    virtual void PreSetUpIon() override;

  protected:
    ButtonPane() {}

    /// Redefines this to return the PushButtonWidget so that all contained
    /// Panes are part of the button.
    virtual SG::Node & GetPaneParent() { return *button_; }

  private:
    PushButtonWidgetPtr button_;

    friend class Parser::Registry;
};

typedef std::shared_ptr<ButtonPane> ButtonPanePtr;
