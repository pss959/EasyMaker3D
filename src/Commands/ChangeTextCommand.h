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
    virtual Str GetDescription() const override;

    /// Sets the new text string.
    void SetNewTextString(const Str &text);

    /// Returns the new text string.
    const Str & GetNewTextString() const { return new_text_string_; }

    /// Sets the new font name.
    void SetNewFontName(const Str &name);

    /// Returns the name of the new font. If this is empty, the font is not
    /// changed.
    const Str & GetNewFontName() const { return new_font_name_; }

    /// Sets the new character spacing.
    void SetNewCharSpacing(float spacing);

    /// Returns the new character spacing.
    float GetNewCharSpacing() const { return new_char_spacing_; }

  protected:
    ChangeTextCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Str>   new_text_string_;
    Parser::TField<Str>   new_font_name_;
    Parser::TField<float> new_char_spacing_;
    ///@}

    friend class Parser::Registry;
};
