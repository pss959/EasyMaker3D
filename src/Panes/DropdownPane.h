//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Panes/BoxPane.h"
#include "Panes/IPaneInteractor.h"
#include "Panes/TextPane.h"
#include "Util/Memory.h"
#include "Util/Notifier.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ButtonPane);
DECL_SHARED_PTR(DropdownPane);
DECL_SHARED_PTR(ScrollingPane);

/// DropdownPane is a derived BoxPane that implements an interactive dropdown
/// with text choices. It normally shows a ButtonPane with the text of the
/// current choice. When the button is activated, the DropdownPane displays a
/// ScrollingPane with all of the available choices.
///
/// \ingroup Panes
class DropdownPane : public BoxPane, public IPaneInteractor {
  public:
    /// Returns a Notifier that is invoked when the user selects a new choice.
    /// It is passed the new choice string.
    Util::Notifier<const Str &> & GetChoiceChanged() {
        return choice_changed_;
    }

    /// Sets the dropdown to contain the given choice strings and the starting
    /// index into them.
    void SetChoices(const StrVec &choices, size_t index);

    /// Sets the index of the current choice. Notifies only if \p should_notify
    /// is true.
    void SetChoice(size_t index, bool should_notify = false);

    /// Sets the current choice to the one matching the given string. Notifies
    /// only if \p should_notify is true. Asserts if the choice is not a valid
    /// one.
    void SetChoiceFromString(const Str &choice, bool should_notify = false);

    /// Returns the current choice. This will be "." until valid choices are
    /// set.
    const Str & GetChoice() const { return choice_; }

    /// Returns the index of the current choice.
    int GetChoiceIndex() const { return choice_index_; }

    /// Returns the Pane used to display the menu of choices. Primarily for
    /// testing and debugging.
    const ScrollingPane & GetMenuPane() const;

    virtual void PostSetUpIon() override;

    // IPaneInteractor interface.
    virtual IPaneInteractor * GetInteractor() override { return this; }
    virtual ClickableWidgetPtr GetActivationWidget() const override;
    virtual BorderPtr GetFocusBorder() const override;
    virtual void Activate() override;
    virtual void Deactivate() override;
    virtual bool IsActive() const override;
    virtual bool HandleEvent(const Event &event) override;

    /// Redefines this to add nothing, since nothing inside the DropdownPane
    /// should be focused individually.
    virtual void GetFocusableSubPanes(PaneVec &panes) const override {};

  protected:
    DropdownPane() {}

    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;
    virtual void CreationDone() override;

    /// Redefines this to use the size of the largest choice string.
    virtual Vector2f ComputeBaseSize() const override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::VField<Str> choices_;
    Parser::TField<int> choice_index_;
    ///@}

    /// Notifies when the choice changes.
    Util::Notifier<const Str &> choice_changed_;

    /// Current choice string. Empty when no valid choice.
    Str                         choice_;

    /// TextPane displaying current choice.
    TextPanePtr                 text_pane_;

    /// ButtonPane that is clicked to show or hide the choice menu.
    ButtonPanePtr               activation_button_pane_;

    /// ScrollingPane used to display menu of choices.
    ScrollingPanePtr            menu_pane_;

    /// ButtonPane used to represent a menu choice. A clone is made for each
    /// choice in the menu.
    ButtonPanePtr               menu_button_pane_;

    /// Cloned ButtonPane for each menu choice.
    std::vector<ButtonPanePtr>  menu_button_panes_;

    /// This width is added to the width of the widest menu choice string to
    /// get the full base size of the DropdownPane.
    float                       menu_extra_width_ = 0;

    /// Updates menu choice buttons when necessary.
    void UpdateMenuPane_();

    void ChoiceButtonClicked_(size_t index);

    friend class Parser::Registry;
};
