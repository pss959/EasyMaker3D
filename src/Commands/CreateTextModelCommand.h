#pragma once

#include "Commands/CreateModelCommand.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(CreateTextModelCommand);

/// CreateTextModelCommand is used to create a TextModel.
///
/// \ingroup Commands
class CreateTextModelCommand : public CreateModelCommand {
  public:
    virtual Str GetDescription() const override;

    /// Sets the text string to display.
    void SetText(const Str &text) { text_ = text; }

    /// Returns the text string.
    const Str & GetText() const { return text_; }

  protected:
    CreateTextModelCommand() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Str> text_;
    ///@}

    friend class Parser::Registry;
};
