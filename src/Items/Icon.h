#pragma once

#include <memory>

#include "Enums/Action.h"
#include "Parser/Object.h"

namespace Parser { class Registry; }

/// The Icon class represents a 3D icon button (pushbutton or toggle
/// button). It stores the associated Action that the button
/// implements. Instances of this class are specified in Shelf items.
class Icon : public Parser::Object {
  public:
    virtual void AddFields() override;

    /// Returns true if the Icon represents a toggle button as opposed to a
    /// regular pushbutton.
    bool IsToggle() const { return is_toggle_; }

    /// Returns the Action associated with the icon.
    Action GetAction() const { return action_; }

  protected:
    Icon() {}

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<bool>      is_toggle_{"is_toggle", false};
    Parser::EnumField<Action> action_{"action", Action::kNone};
    ///@}

    friend class Parser::Registry;
};

typedef std::shared_ptr<Icon> IconPtr;
