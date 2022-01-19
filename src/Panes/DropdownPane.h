#pragma once

#include <memory>

#include "Panes/BoxPane.h"
#include "Util/Notifier.h"

namespace Parser { class Registry; }

/// DropdownPane is a derived BoxPane that implements an interactive dropdown
/// with text choices. It normally shows a ButtonPane with the text of the
/// current choice. When the button is activated, the DropdownPane displays a
/// ScrollingPane with all of the available choices.
class DropdownPane : public BoxPane {
  public:
    /// Returns a Notifier that is invoked when the user selects a new choice.
    /// It is passed the new choice string.
    Util::Notifier<const std::string &> & GetChoiceChanged() {
        return choice_changed_;
    }

    /// Sets the dropdown to contain the given choice strings.
    void SetChoices(const std::vector<std::string> &choices);

    /// Returns the current choice.
    const std::string & GetChoice() const { return choice_; }

    virtual void PostSetUpIon() override;

    virtual void Activate()   override;
    virtual void Deactivate() override;
    virtual bool HandleEvent(const Event &event) override;

  protected:
    DropdownPane() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::VField<std::string> choices_{"choices"};
    Parser::TField<int>         choice_index_{"choice_index", -1};
    ///@}

    /// Notifies when the choice changes.
    Util::Notifier<const std::string &> choice_changed_;

    /// Current choice string. Empty when no valid choice.
    std::string choice_;

    friend class Parser::Registry;
};

typedef std::shared_ptr<DropdownPane> DropdownPanePtr;
