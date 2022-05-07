#pragma once

#include "Memory.h"
#include "Panes/BoxPane.h"
#include "Panes/IPaneInteractor.h"
#include "Panes/TextPane.h"
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
    Util::Notifier<const std::string &> & GetChoiceChanged() {
        return choice_changed_;
    }

    /// Sets the dropdown to contain the given choice strings and the starting
    /// index into them.
    void SetChoices(const std::vector<std::string> &choices, size_t index);

    /// Sets the index of the current choice. Does not notify.
    void SetChoice(size_t index);

    /// Sets the current choice to the one matching the given string. Does not
    /// notify. Asserts if the choice is not a valid one.
    void SetChoiceFromString(const std::string &choice);

    /// Returns the current choice.
    const std::string & GetChoice() const { return choice_; }

    /// Returns the index of the current choice.
    int GetChoiceIndex() const { return choice_index_; }

    /// Returns the Pane used to display the menu of choices. Primarily for
    /// testing and debugging.
    const ScrollingPane & GetMenuPane() const;

    virtual void PostSetUpIon() override;

    // IPaneInteractor interface.
    virtual IPaneInteractor * GetInteractor() override { return this; }
    virtual ClickableWidgetPtr GetActivationWidget() const override;
    virtual bool CanFocus() const override;
    virtual void SetFocus(bool is_focused) override;
    virtual void Activate() override;
    virtual bool IsActive() const override;
    virtual bool HandleEvent(const Event &event) override;

    /// Redefines this to return an empty vector, since nothing inside the
    /// DropdownPane should be focused individually.
    virtual std::vector<PanePtr>
    GetPotentialInteractiveSubPanes() const override {
        return std::vector<PanePtr>();
    }

  protected:
    DropdownPane() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void CreationDone() override;

    /// Redefines this to use the size of the largest choice string.
    virtual Vector2f ComputeBaseSize() const override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::VField<std::string> choices_{"choices"};
    Parser::TField<int>         choice_index_{"choice_index", -1};
    ///@}

    /// Notifies when the choice changes.
    Util::Notifier<const std::string &> choice_changed_;

    /// Current choice string. Empty when no valid choice.
    std::string      choice_;

    /// TextPane displaying current choice.
    TextPanePtr      text_pane_;

    /// ScrollingPane used to display menu of choices.
    ScrollingPanePtr menu_pane_;

    /// ButtonPane used to represent a menu choice. A clone is made for each
    /// choice in the menu.
    ButtonPanePtr    menu_button_pane_;

    /// This width is added to the width of the widest menu choice string to
    /// get the full base size of the DropdownPane.
    float            menu_extra_width_ = 0;

    /// Updates menu choice buttons when necessary.
    void UpdateMenuPane_();

    void ChoiceButtonClicked_(size_t index);

    friend class Parser::Registry;
};
