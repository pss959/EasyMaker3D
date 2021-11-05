#pragma once

#include <string>

#include "Panels/Panel.h"

namespace Parser { class Registry; }

/// SessionPanel is a derived Panel class that implements session management.
class SessionPanel : public Panel {
  protected:
    SessionPanel() {}

    virtual void ProcessButton(const std::string &name) override;

  private:
    friend class Parser::Registry;
};
