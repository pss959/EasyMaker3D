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

    /// Defines this to change the size of the text if resizing is enabled.
    virtual void SetSize(const Vector2f &size) override;

  protected:
    TextPane() {}

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string> text_{"text"};
    ///@}

    SG::TextNodePtr text_node_;

    friend class Parser::Registry;
};

typedef std::shared_ptr<TextPane> TextPanePtr;
