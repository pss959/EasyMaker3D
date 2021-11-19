#pragma once

#include <memory>

#include "Panes/Pane.h"
#include "SG/LayoutOptions.h"
#include "SG/Typedefs.h"

namespace Parser { class Registry; }

/// TextPane is a derived Pane that displays a text string. The text is a fixed
/// font and size specified in the "font_name" and "font_size" fields. The text
/// is centered within the pane area unless the "halignment" or "valignment"
/// fields are set. The "padding" field is used for non-center alignments.
class TextPane : public Pane {
  public:
    typedef SG::LayoutOptions::HAlignment HAlignment;
    typedef SG::LayoutOptions::VAlignment VAlignment;

    virtual void AddFields() override;

    /// Sets the text string.
    void SetText(const std::string &text);

    virtual void PreSetUpIon() override;
    virtual void PostSetUpIon() override;

    /// Defines this to also change the size of the text if resizing is
    /// enabled.
    virtual void SetSize(const Vector2f &size) override;

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

    SG::TextNodePtr text_node_;
    Vector2f        text_size_;

    /// Updates the text_size_ variable and adjusts the minimum size of the
    /// TextPane if necessary.
    void UpdateTextSize_();

    /// Sets the target size in the SG::LayoutOptions to make the text fit
    /// properly in the given size.
    void SetTargetSize_(const Vector2f &size);

    /// Sets the target point in the SG::LayoutOptions based on the alignment
    /// settings.
    void SetTargetPoint_();

    /// Returns the size of text assuming it does not resize.
    Vector2f GetFixedSize_() const;

    friend class Parser::Registry;
};

typedef std::shared_ptr<TextPane> TextPanePtr;
