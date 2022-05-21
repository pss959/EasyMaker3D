#pragma once

#include "Base/Memory.h"
#include "Commands/MultiModelCommand.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ChangeTextCommand);

/// ChangeTextCommand is used to change the data in one or more TextModel
/// instances.
///
/// \ingroup Commands
class ChangeTextCommand : public MultiModelCommand {
  public:
    virtual std::string GetDescription() const override;

    /// Sets the new text string.
    void SetNewTextString(const std::string &text);

    /// Returns the new text string.
    const std::string & GetNewTextString() const { return new_text_string_; }

    /// Sets the new font name.
    void SetNewFontName(const std::string &name);

    /// Returns the name of the new font. If this is empty, the font is not
    /// changed.
    const std::string & GetNewFontName() const { return new_font_name_; }

    /// Sets the new character spacing.
    void SetNewCharSpacing(float spacing);

    /// Returns the new character spacing.
    float GetNewCharSpacing() const { return new_char_spacing_; }

  protected:
    ChangeTextCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string> new_text_string_{"new_text_string"};
    Parser::TField<std::string> new_font_name_{"new_font_name"};
    Parser::TField<float>       new_char_spacing_{"new_char_spacing", 1};
    ///@}

    friend class Parser::Registry;
};
