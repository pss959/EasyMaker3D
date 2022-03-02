#pragma once

#include <memory>

#include "Panes/BoxPane.h"

namespace Parser { class Registry; }

/// TextInputPane is a derived BoxPane that supports interactive editing of
/// displayed text.
class TextInputPane : public BoxPane {
  public:
    /// Typedef for a function that is used to determine whether the current
    /// input text is valid. The current text is supplied.
    typedef std::function<bool(const std::string &)> ValidationFunc;

    /// Sets a function that is used to determine whether the current text is
    /// valid. If this function is not null and returns false, the background
    /// of the TextInputPane is changed to indicate an error.
    void SetValidationFunc(const ValidationFunc &func);

    /// Sets the initial text to display.
    void SetInitialText(const std::string &text);

    /// Returns the current text. This will be empty until the TextInputPane is
    /// fully set up.
    std::string GetText() const;

    virtual bool IsInteractive()        const override { return true; }
    virtual bool IsInteractionEnabled() const override { return true; }
    virtual void Activate()   override;
    virtual void Deactivate() override;
    virtual bool HandleEvent(const Event &event) override;

  protected:
    TextInputPane();

    virtual void AddFields() override;
    virtual void CreationDone() override;

    /// Redefines this to also update the character width.
    virtual void SizeChanged(const Pane &initiating_pane) override;

  private:
    class Impl_;
    class StateStack_;
    std::unique_ptr<Impl_> impl_;

    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string> initial_text_{"initial_text", "."};
    ///@}

    friend class Parser::Registry;
};

typedef std::shared_ptr<TextInputPane> TextInputPanePtr;
