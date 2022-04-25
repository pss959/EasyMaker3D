#pragma once

#include "Memory.h"
#include "Panels/Panel.h"

DECL_SHARED_PTR(TestPanel);

namespace Parser { class Registry; }

/// TestPanel is a derived Panel class used for testing stuff.
///
/// \ingroup Panels
class TestPanel : public Panel {
  protected:
    TestPanel() {}

    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

  private:
    void FindButtonPanes_(const PanePtr &pane);

    friend class Parser::Registry;
};
