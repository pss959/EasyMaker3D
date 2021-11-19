#pragma once

#include "Panels/Panel.h"

namespace Parser { class Registry; }

/// TestPanel is a derived Panel class used for testing stuff.
class TestPanel : public Panel {
  protected:
    TestPanel() {}

    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

  private:
    friend class Parser::Registry;
};

typedef std::shared_ptr<TestPanel> TestPanelPtr;
