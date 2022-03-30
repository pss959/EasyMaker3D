#pragma once

#include <string>

#include "Commands/CreateModelCommand.h"
#include "Memory.h"

DECL_SHARED_PTR(CreateTextModelCommand);

/// CreateTextModelCommand is used to create a TextModel.
///
/// \ingroup Commands
class CreateTextModelCommand : public CreateModelCommand {
  public:
    virtual std::string GetDescription() const override;

    /// Sets the text string to display.
    void SetText(const std::string &text) { text_ = text; }

    /// Returns the text string.
    const std::string & GetText() const { return text_; }

  protected:
    CreateTextModelCommand() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string> text_{"text", "A"};
    ///@}

    friend class Parser::Registry;
};
