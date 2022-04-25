#pragma once

#include <string>
#include <unordered_map>

#include "Memory.h"
#include "Enums/Action.h"
#include "Panels/Panel.h"

DECL_SHARED_PTR(ActionPanel);
DECL_SHARED_PTR(ButtonPane);

namespace Parser { class Registry; }

/// ActionPanel is a derived Panel class that allows the user to choose an
/// action to attach to a radial menu button.
///
/// \ingroup Panels
class ActionPanel : public Panel {
  public:
    /// Sets the Action that is currently attached to the button.
    void SetAction(Action action);

    /// Returns the Action that was selected by the user.
    Action GetAction() const { return current_action_; }

  protected:
    ActionPanel() {}

    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

  private:
    /// Map from action to button.
    std::unordered_map<Action, ButtonPanePtr> button_map_;

    Action current_action_ = Action::kNone;

    void ChangeHighlight_(Action action, bool state);

    friend class Parser::Registry;
};
