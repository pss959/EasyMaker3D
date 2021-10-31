#pragma once

#include <memory>

#include "Panes/SolidPane.h"
#include "Widgets/PushButtonWidget.h"

namespace Parser { class Registry; }

/// ButtonPane is a derived SolidPane that acts as a pushbutton.
class ButtonPane : public SolidPane {
  public:
    virtual void PreSetUpIon() override;

  protected:
    ButtonPane() {}

  private:
    PushButtonWidgetPtr button_;

    friend class Parser::Registry;
};

typedef std::shared_ptr<ButtonPane> ButtonPanePtr;
