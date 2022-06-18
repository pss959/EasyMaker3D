#pragma once

#include "Panels/Panel.h"

DECL_SHARED_PTR(BoxPane);
DECL_SHARED_PTR(KeyboardPanel);

namespace Parser { class Registry; }

/// KeyboardPanel is a derived Panel class that acts as a virtual keyboard.
///
/// \ingroup Panels
class KeyboardPanel : public Panel {
  protected:
    KeyboardPanel() {}

    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

  private:
    friend class Parser::Registry;
};
