#pragma once

#include <string>

#include "Enums/Action.h"
#include "Panels/Panel.h"
#include "Panes/ButtonPane.h"

namespace Parser { class Registry; }

/// ActionPanel is a derived Panel class that allows the user to choose an
/// action to attach to a radial menu button.
class ActionPanel : public Panel {
  public:
    /// Sets the Action that is currently attached to the button.
    void SetAction(Action action) { current_action_ = action; }

    /// Returns the Action that was selected by the user.
    Action GetAction() const { return current_action_; }

  protected:
    ActionPanel() {}

    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

  private:
    Action        current_action_ = Action::kNone;
    ButtonPanePtr current_button_;

    void ButtonClicked_(Action action, const ButtonPanePtr &but);

    friend class Parser::Registry;
};

typedef std::shared_ptr<ActionPanel> ActionPanelPtr;
