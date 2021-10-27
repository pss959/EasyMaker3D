#pragma once

#include <memory>

#include "Panes/Pane.h"

namespace Parser { class Registry; }

/// TextPane is a derived Pane that displays a text string.
class TextPane : public Pane {
  public:
    virtual void AddFields() override;

    /// Sets the text string.
    void SetText(const std::string &text);

    virtual void PreSetUpIon() override;

  protected:
    TextPane() {}

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string> text_{"text"};
    ///@}

    friend class Parser::Registry;
};

typedef std::shared_ptr<TextPane> TextPanePtr;
