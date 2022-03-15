#pragma once

#include <string>

#include "Enums/Action.h"
#include "Panels/Panel.h"

namespace Parser { class Registry; }

/// ActionPanel is a derived Panel class that allows the user to choose an
/// action to attach to a radial menu button.
class ActionPanel : public Panel {
  protected:
    ActionPanel() {}

    /// Sets the Action that is currently attached to the button.
    void SetAction(Action action) { current_action_ = action; }

    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

  private:
    Action current_action_ = Action::kNone;

    void Accept_();

    friend class Parser::Registry;
};

typedef std::shared_ptr<ActionPanel> ActionPanelPtr;
