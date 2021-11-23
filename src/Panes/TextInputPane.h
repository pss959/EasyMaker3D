#pragma once

#include <memory>

#include "Panes/BoxPane.h"
#include "Panes/TextPane.h"
#include "SG/Typedefs.h"

struct ClickInfo;
namespace Parser { class Registry; }

/// TextInputPane is a derived BoxPane that supports interactive editing of
/// displayed text.
class TextInputPane : public BoxPane {
  public:
    virtual void AddFields() override;

    // Sets the initial text to display.
    void SetInitialText(const std::string &text);

    virtual void PreSetUpIon() override;
    virtual void PostSetUpIon() override;

    virtual bool IsInteractive()        const override { return true; }
    virtual bool IsInteractionEnabled() const override { return true; }
    virtual void Activate()   override;
    virtual void Deactivate() override;
    virtual bool HandleEvent(const Event &event) override;

  protected:
    TextInputPane() {}

    /// Redefines this to also update the character width.
    virtual void ProcessSizeChange() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string> initial_text_{"initial_text"};
    ///@}

    /// True when the pane is active (editing).
    bool is_active_ = false;

    /// Width of a single text character in monospace font.
    float char_width_ = 0;

    /// Position of the cursor relative to the current text.
    size_t cursor_pos_ = 0;

    /// TextPane used to display the current text.
    TextPanePtr text_pane_;

    void UpdateCharWidth_();
    void SetBackgroundColor_(const std::string &color_name);
    void ShowCursor_(bool show);
    void MoveCursor_(size_t new_pos);
    void ProcessClick_(const ClickInfo &info);

    friend class Parser::Registry;
};

typedef std::shared_ptr<TextInputPane> TextInputPanePtr;
