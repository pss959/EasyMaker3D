#pragma once

#include "Math/Color.h"
#include "Panes/LeafPane.h"
#include "SG/LayoutOptions.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }
namespace SG { DECL_SHARED_PTR(TextNode); }

DECL_SHARED_PTR(TextPane);

/// TextPane is a derived LeafPane that displays a text string. The text is
/// centered within the pane area unless the "halignment" or "valignment"
/// fields are set.
///
/// TextPane computes its base size unlike other Panes. Because text is
/// horizontal, the TextPane sets the height based on the font size, the number
/// of lines of text, the line spacing, and the padding. The width is computed
/// to maintain the proper aspect ratio of the text, taking padding into
/// account.
///
/// \ingroup Panes
class TextPane : public LeafPane {
  public:
    using HAlignment = SG::LayoutOptions::HAlignment;
    using VAlignment = SG::LayoutOptions::VAlignment;

    /// Sets the text string. If the string is empty, this disables the
    /// SG::TextNode.
    void SetText(const Str &text);

    /// Returns the current text string.
    const Str & GetText() const { return text_; }

    /// Sets the color to use for the text.
    void SetColor(const Color &color);

    /// Returns the color to use for the text.
    const Color & GetColor() const { return color_; }

    /// Sets the name of the font to use for the text.
    void SetFontName(const Str &font_name);

    /// Returns the name of the font to use for the text.
    const Str & GetFontName() const { return font_name_; }

    /// Sets the font size to use for the text.
    void SetFontSize(float font_size);

    /// Returns the font size to use for the text.
    float GetFontSize() const { return font_size_; }

    /// Sets the character spacing for the text.
    void SetCharacterSpacing(float spacing);

    /// Returns the character spacing used for the text.
    float GetCharacterSpacing() const { return char_spacing_; }

    /// Sets the offset to use for the text. This offset is added to the
    /// translation computed for the text and is independent of alignment and
    /// sizing.
    void SetOffset(const Vector2f &offset) { offset_ = offset; }

    /// Returns the offset to use for the text.
    const Vector2f & GetOffset() { return offset_; }

    /// Sets a flag indicating whether the text is resizable in response to
    /// expanding height.
    void SetTextResizable(bool resizable) { resize_text_ = resizable; }

    /// Returns a flag indicating whether the text is resizable in response to
    /// expanding height. The default is true.
    bool IsTextResizable() const { return resize_text_; }

    /// Redefines this to update the size and placement of the text if
    /// necesssary.
    virtual void UpdateForLayoutSize(const Vector2f &size) override;

    /// Returns the current size of the text as it appears in the pane (not
    /// including padding). This will be zero until SetSize() is called.
    const Vector2f & GetTextSize() const { return text_size_; }

    virtual void PostSetUpIon() override;

    virtual Str ToString(bool is_brief) const override;

  protected:
    TextPane() {}

    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;
    virtual void CreationDone() override;

    /// Redefines this to use the font size, padding, etc.
    virtual Vector2f ComputeBaseSize() const override;

    /// Redefines this to also indicate that the TextPane size may have changed.
    virtual bool ProcessChange(SG::Change change, const Object &obj) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Str>           text_;
    Parser::TField<Color>         color_;
    Parser::TField<Str>           font_name_;
    Parser::TField<float>         font_size_;
    Parser::TField<bool>          resize_text_;
    Parser::EnumField<HAlignment> halignment_;
    Parser::EnumField<VAlignment> valignment_;
    Parser::TField<float>         char_spacing_;
    Parser::TField<float>         line_spacing_;
    Parser::TField<float>         padding_;
    Parser::TField<Vector2f>      offset_;
    ///@}

    /// TextNode that renders the text.
    SG::TextNodePtr text_node_;

    /// Unpadded base size returned by ComputeUnpaddedBaseSize_(). This is used
    /// for computing both the base size and the text scaling.
    mutable Vector2f unpadded_base_size_;

    /// Size of the text after it has been scaled to fit in the pane.
    mutable Vector2f text_size_{0, 0};

    Vector2f ComputeUnpaddedBaseSize_() const;

    /// Updates the scale and translation of the text based on the given pane
    /// size and alignment and padding settings.
    void UpdateTextTransform_(const Vector2f &pane_size) const;
    Vector3f ComputeTextScale_(const Vector2f &pane_size,
                               const Vector2f &text_size) const;
    Vector3f ComputeTextTranslation_(const Vector2f &pane_size) const;

    /// Computes SG::LayoutOptions glyph spacing based on the given character
    /// spacing and the current font size.
    float GetGlyphSpacing_(float char_spacing) const;

    friend class Parser::Registry;
};
