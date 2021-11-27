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

    virtual void AddFields() override;

    /// Sets the text string.
    void SetText(const std::string &text);

    /// Returns the current text string.
    const std::string & GetText() const { return text_; }

    /// Redefines this to update the size and placement of the text if
    /// necesssary.
    virtual void SetSize(const Vector2f &size) override;

    /// Returns the current size of the text as it appears in the pane (not
    /// including padding). This will be zero until PostSetUpIon() is called.
    const Vector2f & GetTextSize() const { return text_size_; }

    virtual void PreSetUpIon() override;
    virtual void PostSetUpIon() override;

    virtual std::string ToString() const override;

  protected:
    TextPane() {}

    /// Redefines this to use the computed text size if it is known.
    virtual Vector2f ComputeMinSize() const;

    /// Redefines this to also indicate that the TextPane size has changed.
    virtual void ProcessChange(SG::Change change) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string>   text_{"text"};
    Parser::TField<std::string>   font_name_{"font_name", "Arial"};
    Parser::EnumField<HAlignment> halignment_{"halignment",
                                              HAlignment::kAlignHCenter};
    Parser::EnumField<VAlignment> valignment_{"valignment",
                                              VAlignment::kAlignVCenter};
    Parser::TField<float>         padding_{"padding", 0};
    ///@}

    /// TextNode that renders the text.
    SG::TextNodePtr text_node_;

    /// Size of the text after it has been scaled to fit in the pane.
    Vector2f text_size_;

    /// Updates the scale and translation of the text based on the current pane
    /// size and alignment and padding settings.
    void UpdateTextTransform_();

    Vector3f ComputeTextScale_();
    Vector3f ComputeTextTranslation_();

    friend class Parser::Registry;
};

typedef std::shared_ptr<TextPane> TextPanePtr;
