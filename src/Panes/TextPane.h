#pragma once

#include <memory>

#include "Panes/Pane.h"
#include "SG/Typedefs.h"

namespace Parser { class Registry; }

/// TextPane is a derived Pane that displays a text string.
class TextPane : public Pane {
  public:
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

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string> text_{"text"};
    ///@}

    SG::TextNodePtr text_node_;
    Vector2f text_size_;

    /// Returns the size of text assuming it does not resize.
    Vector2f GetFixedSize_() const;

    friend class Parser::Registry;
};

typedef std::shared_ptr<TextPane> TextPanePtr;
