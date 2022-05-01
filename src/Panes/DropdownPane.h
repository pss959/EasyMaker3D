#pragma once

#include "Memory.h"
#include "Panes/BoxPane.h"
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
class DropdownPane : public BoxPane {
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

    /// Returns the Pane used to display the choices. Primarily for testing and
    /// debugging.
    const ScrollingPane & GetChoicePane() const;

    virtual void PostSetUpIon() override;
    virtual bool IsInteractive()        const override { return true; }
    virtual bool IsInteractionEnabled() const override { return true; }
    virtual void Activate()   override;
    virtual void Deactivate() override;
    virtual bool HandleEvent(const Event &event) override;

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

    /// ScrollingPane used to display choices.
    ScrollingPanePtr choice_pane_;

    /// ButtonPane used to represent a choice. Clones are made for choices.
    ButtonPanePtr    choice_button_pane_;

    /// This saves the width of the ButtonPane showing the current choice when
    /// no text is present. This is used to compute the base size.
    float            button_extra_width_ = 0;

    /// Updates choice buttons when necessary.
    void UpdateChoicePane_();

    void ChoiceButtonClicked_(size_t index);

    friend class Parser::Registry;
};
