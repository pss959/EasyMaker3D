#pragma once

#include <memory>

#include "Panes/Pane.h"
#include "SG/LayoutOptions.h"
#include "SG/Typedefs.h"

namespace Parser { class Registry; }

/// TextPane is a derived Pane that displays a text string. The text is
/// centered within the pane area unless the "halignment" or "valignment"
/// fields are set. The text is sized so that it fills either the width or
/// height of the pane (minus the padding) while maintaining the proper aspect
/// ratio.
///
/// The minimum size of a TextPane uses the Defaults::kMinimumPaneTextHeight
/// value in addition to the base size.
class TextPane : public Pane {
  public:
    typedef SG::LayoutOptions::HAlignment HAlignment;
    typedef SG::LayoutOptions::VAlignment VAlignment;

    /// Sets the text string.
    void SetText(const std::string &text);

    /// Sets the color to use for the text.
    void SetColor(const Color &color);

    /// Sets the name of the font to use for the text.
    void SetFontName(const std::string &font_name);

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
    virtual Vector2f ComputeBaseSize() const override;

    /// Redefines this to also indicate that the TextPane size may have changed.
    virtual bool ProcessChange(SG::Change change, const Object &obj) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string>   text_{"text", "."};
    Parser::TField<Color>         color_{"color"};
    Parser::TField<std::string>   font_name_{"font_name", "Arial"};
    Parser::EnumField<HAlignment> halignment_{"halignment",
                                              HAlignment::kAlignHCenter};
    Parser::EnumField<VAlignment> valignment_{"valignment",
                                              VAlignment::kAlignVCenter};
    Parser::TField<float>         padding_{"padding", 0};
    Parser::TField<Vector2f>      offset_{"offset", {0, 0}};
    ///@}

    /// TextNode that renders the text.
    SG::TextNodePtr text_node_;

    /// Size of the text after it has been scaled to fit in the pane.
    Vector2f text_size_;

    /// Updates the scale and translation of the text based on the given pane
    /// size and alignment and padding settings.
    void UpdateTextTransform_(const Vector2f &pane_size);

    Vector3f ComputeTextScale_();
    Vector3f ComputeTextTranslation_();

    friend class Parser::Registry;
};

typedef std::shared_ptr<TextPane> TextPanePtr;
