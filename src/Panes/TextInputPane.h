#pragma once

#include <memory>

#include "Panes/BoxPane.h"
#include "SG/Typedefs.h"

namespace Parser { class Registry; }

/// TextInputPane is a derived BoxPane that supports interactive editing of
/// displayed text.
class TextInputPane : public BoxPane {
  public:
    virtual void AddFields() override;

    virtual void PreSetUpIon() override;
    virtual void PostSetUpIon() override;

    virtual bool IsInteractive()        const override { return true; }
    virtual bool IsInteractionEnabled() const override { return true; }
    virtual void Activate() override;

  protected:
    TextInputPane() {}

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string> initial_text_{"initial_text"};
    ///@}

    friend class Parser::Registry;
};

typedef std::shared_ptr<TextInputPane> TextInputPanePtr;
