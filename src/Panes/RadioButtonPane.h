#pragma once

#include <memory>
#include <vector>

#include "Panes/Pane.h"

namespace Parser { class Registry; }

class RadioButtonPane;
typedef std::shared_ptr<RadioButtonPane> RadioButtonPanePtr;

/// RadioButtonPane is a derived Pane that implements an interactive
/// radio button. Call CreateGroup() to make a set of RadioButtonPane instances
/// define mutually exclusive behavior.
class RadioButtonPane : public Pane {
  public:
    /// Returns the current state of the button.
    bool GetState() const { return state_; }

    /// Sets the state of the button. If this button is part of a group,
    /// setting the state to true will turn off all other buttons in the group,
    /// but setting the state to false will have no effect.
    void SetState(bool new_state);

    /// Creates a group from a collection of RadioButtonPane instances. All
    /// radio buttons in this group will be mutually exclusive. It is an error
    /// to add a RadioButtonPane to more than one group. The button
    /// corresponding to the selected_index parameter will be set to true.
    static void CreateGroup(const std::vector<RadioButtonPanePtr> &buttons,
                            size_t selected_index);

    virtual void PostSetUpIon() override;

    virtual bool IsInteractive()        const override { return true; }
    virtual bool IsInteractionEnabled() const override { return true; }
    virtual void Activate()   override;
    virtual void Deactivate() override;
    virtual bool HandleEvent(const Event &event) override;

  protected:
    RadioButtonPane() {}

    virtual void AddFields() override;

  private:
    /// Allows individual RadioButtonPane instances to be grouped.
    struct Group_ {
        /// Stores the buttons as weak pointers so there are no reference
        /// cycles.
        std::vector<std::weak_ptr<RadioButtonPane>> buttons;
    };

    /// \name Parsed Fields
    ///@{
    Parser::TField<bool> state_{"state", false};
    ///@}

    /// Group this button is part of.
    std::shared_ptr<Group_> group_;

    void Toggle_();
    void UpdateState_();

    friend class Parser::Registry;
};
