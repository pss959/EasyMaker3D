//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <vector>

#include "Panes/IPaneInteractor.h"
#include "Panes/LeafPane.h"
#include "Util/Memory.h"
#include "Util/Notifier.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(PushButtonWidget);
DECL_SHARED_PTR(RadioButtonPane);

/// RadioButtonPane is a derived LeafPane that implements an interactive
/// radio button. Call CreateGroup() to make a set of RadioButtonPane instances
/// define mutually exclusive behavior.
///
/// \ingroup Panes
class RadioButtonPane : public LeafPane, public IPaneInteractor {
  public:
    /// Returns a Notifier that is invoked when the button changes state. If
    /// the button is part of a group, the function is passed the index of the
    /// button in that group. Otherwise, the index has no meaning.
    Util::Notifier<size_t> & GetStateChanged() { return state_changed_; }

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

    // IPaneInteractor interface.
    virtual IPaneInteractor * GetInteractor() override { return this; }
    virtual ClickableWidgetPtr GetActivationWidget() const override;
    virtual BorderPtr GetFocusBorder() const override;
    virtual void Activate() override;

  protected:
    RadioButtonPane() {}

    virtual void CreationDone() override;
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
    Parser::TField<bool> state_;
    ///@}

    /// Notifies when the button state changes.
    Util::Notifier<size_t> state_changed_;

    /// Group this button is part of.
    std::shared_ptr<Group_> group_;

    /// Index of this button in its group.
    size_t index_in_group_ = 0;

    /// Node that shows the check mark.
    SG::NodePtr         check_;

    /// Associated button.
    PushButtonWidgetPtr button_;

    void Toggle_();
    void UpdateState_();

    friend class Parser::Registry;
};
