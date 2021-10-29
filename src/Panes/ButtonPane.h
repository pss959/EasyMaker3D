#pragma once

#include <memory>

#include "Panes/Pane.h"
#include "SG/Typedefs.h"

namespace Parser { class Registry; }

/// ButtonPane is a derived Pane that acts as a pushbutton.
class ButtonPane : public Pane {
  public:
    virtual void AddFields() override;

    virtual void PreSetUpIon() override;

  protected:
    ButtonPane() {}

  private:
    friend class Parser::Registry;
};

typedef std::shared_ptr<ButtonPane> ButtonPanePtr;
