#pragma once

#include "Memory.h"
#include "Panes/Pane.h"
#include "SG/LayoutOptions.h"

namespace Parser { class Registry; }
namespace SG { DECL_SHARED_PTR(TextNode); }

DECL_SHARED_PTR(TextPane);

/// TextPane is a derived Pane that displays a text string. The text is
/// centered within the pane area unless the "halignment" or "valignment"
/// fields are set.
///
/// TextPane computes its size unline other Panes. Because text is horizontal,
/// it uses the font_size field value as a fixed quantity and adjusts the
/// height based on it, the number of lines of text, the line spacing, and the
/// padding. The width is computed to maintain the proper aspect ratio of the
/// text.
class TextPane : public Pane {
  public:
    typedef SG::LayoutOptions::HAlignment HAlignment;
    typedef SG::LayoutOptions::VAlignment VAlignment;

    /// Sets the text string. If the string is empty, this disables the
    /// SG::TextNode.
    void SetText(const std::string &text);

    /// Sets the color to use for the text.
    void SetColor(const Color &color);

    /// Sets the name of the font to use for the text.
    void SetFontName(const std::string &font_name);

    /// Sets the font size to use for the text.
    void SetFontSize(float font_size);

    /// Sets the offset to use for the text. This offset is added to the
    /// translation computed for the text and is independent of alignment and
    /// sizing.
    void SetOffset(const Vector2f &offset) { offset_ = offset; }

    /// Returns the current text string.
    const std::string & GetText() const { return text_; }

    /// Redefines this to update the size and placement of the text if
    /// necesssary.
    virtual void SetSize(const Vector2f &size) override;

    /// Returns the current size of the text as it appears in the pane (not
    /// including padding). This will be zero until SetSize() is called.
    const Vector2f & GetTextSize() const { return text_size_; }

    virtual std::string ToString() const override;

  protected:
    TextPane() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void CreationDone() override;

    /// Redefines this to use the computed text size if it is known.
    virtual Vector2f ComputeBaseSize() override;

    /// Redefines this to also indicate that the TextPane size may have changed.
    virtual bool ProcessChange(SG::Change change, const Object &obj) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string>   text_{"text", "."};
    Parser::TField<Color>         color_{"color"};
    Parser::TField<std::string>   font_name_{"font_name", "Arial"};
    Parser::TField<float>         font_size_{"font_size", 12};
    Parser::EnumField<HAlignment> halignment_{"halignment",
                                              HAlignment::kAlignHCenter};
    Parser::EnumField<VAlignment> valignment_{"valignment",
                                              VAlignment::kAlignVCenter};
    Parser::TField<float>         line_spacing_{"line_spacing", 1.f};
    Parser::TField<float>         padding_{"padding", 0};
    Parser::TField<Vector2f>      offset_{"offset", {0, 0}};
    ///@}

    /// TextNode that renders the text.
    SG::TextNodePtr text_node_;

    /// Unpadded base size returned by ComputeUnpaddedBaseSize_(). This is used
    /// for computing both the base size and the text scaling.
    mutable Vector2f unpadded_base_size_;

    /// Size of the text after it has been scaled to fit in the pane.
    Vector2f text_size_;

    Vector2f ComputeUnpaddedBaseSize_() const;

    /// Updates the scale and translation of the text based on the given pane
    /// size and alignment and padding settings.
    void UpdateTextTransform_(const Vector2f &pane_size);
    Vector3f ComputeTextScale_(const Vector2f &pane_size,
                               const Vector2f &text_size);
    Vector3f ComputeTextTranslation_(const Vector2f &pane_size);

    friend class Parser::Registry;
};
