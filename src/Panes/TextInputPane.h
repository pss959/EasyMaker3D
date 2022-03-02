#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include "Enums/TextAction.h"
#include "Panes/BoxPane.h"
#include "Panes/TextPane.h"

struct ClickInfo;
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
    void SetValidationFunc(const ValidationFunc &func) {
        validation_func_ = func;
    }

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
    TextInputPane() {}

    virtual void AddFields() override;
    virtual void CreationDone() override;

    /// Redefines this to also update the character width.
    virtual void SizeChanged(const Pane &initiating_pane) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string> initial_text_{"initial_text", "."};
    ///@}

    /// Function to invoke to determine if the current text is valid.
    ValidationFunc validation_func_;

    /// True when the pane is active (editing).
    bool is_active_ = false;

    /// Width of a single text character in monospace font.
    float char_width_ = 0;

    /// Character position of the cursor within the text.
    size_t cursor_pos_ = 0;

    /// Character position of the selection start and end within the text.
    /// There is no selection if these are equal.
    size_t selection_pos_[2]{0, 0};

    /// TextPane used to display the current text.
    TextPanePtr text_pane_;

    /// Stack of edited strings to support undo/redo. This is stored as a
    /// vector because redo requires random access.
    std::vector<std::string> text_stack_;

    /// Index to use for next stack entry.
    size_t stack_index_ = 0;

    /// Maps key string sequences to actions.
    static std::unordered_map<std::string, TextAction> s_action_map_;

    static void InitActionMap_();

    void ProcessAction_(TextAction action);
    void InsertChars(const std::string &chars);
    void DeleteChars_(size_t start_pos, int count, int cursor_motion);
    void ChangeText_(const std::string &new_text, bool add_to_stack = true);
    void ChangeSelection_(size_t start, size_t end);
    void UpdateCharWidth_();
    void UpdateBackgroundColor_();
    void ShowCursor_(bool show);
    void MoveCursor_(size_t new_pos);
    void ProcessClick_(const ClickInfo &info);

    /// Converts a character position to a local X coordinate value.
    float CharPosToX_(size_t pos) const;

    /// Converts a local X coordinate to a character position.
    size_t XToCharPos_(float x) const;

    friend class Parser::Registry;
};

typedef std::shared_ptr<TextInputPane> TextInputPanePtr;
