#pragma once

#include <string>

#include "Base/Memory.h"
#include "Models/PrimitiveModel.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(TextModel);

/// TextModel represents a Model displaying 3D text. The text is extruded along
/// the +Y axis with a default height of 2 units.
///
/// \ingroup Models
class TextModel : public PrimitiveModel {
  public:
    /// Sets the text string to use.
    void SetTextString(const std::string &text);

    /// Returns the current text string.
    const std::string & GetTextString() const { return text_; }

    /// Sets the name to the font to use. An empty name (the default) results
    /// in the default font being used.
    void SetFontName(const std::string &name);

    /// Returns the name of the current font.
    const std::string & GetFontName() const { return font_name_; }

    /// Sets the character spacing.
    void SetCharSpacing(float spacing);

    /// Returns the current character spacing.
    float GetCharSpacing() const { return char_spacing_; }

    /// TextModel responds to complexity.
    virtual bool CanSetComplexity() const override { return true; }

  protected:
    TextModel() {}
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual TriMesh BuildMesh() override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::TField<std::string> text_;
    Parser::TField<std::string> font_name_;
    Parser::TField<float>       char_spacing_;
    ///@}

    /// Validates the given text string, asserting on error.
    void ValidateText_(const std::string &font_name, const std::string &text);

    friend class Parser::Registry;
};
