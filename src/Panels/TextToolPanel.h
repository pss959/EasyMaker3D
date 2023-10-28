#pragma once

#include <string>

#include "Panels/ToolPanel.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(DropdownPane);
DECL_SHARED_PTR(SliderPane);
DECL_SHARED_PTR(TextToolPanel);
DECL_SHARED_PTR(TextInputPane);
DECL_SHARED_PTR(TextPane);

namespace Parser { class Registry; }

/// TextToolPanel is a derived ToolPanel class thatis used by the TextTool for
/// interactive editing of a TextModel.
///
/// ReportChange keys: "Apply" (immediate), when the Apply button is clicked.
///
/// \ingroup Panels
class TextToolPanel : public ToolPanel {
  public:
    /// Initializes the text string, font name, and character spacing.
    void SetValues(const Str &text, const Str &font_name, float char_spacing);

    /// Returns the current text string after possible editing.
    Str GetTextString() const;

    /// Returns the current font name after possible editing.
    const Str & GetFontName() const;

    /// Returns the character spacing after possible editing.
    float GetCharSpacing() const;

  protected:
    TextToolPanel() {}

    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

  private:
    TextInputPanePtr text_pane_;
    DropdownPanePtr  font_pane_;
    SliderPanePtr    spacing_pane_;
    TextPanePtr      display_pane_;
    TextPanePtr      message_pane_;

    // Original settings to be able to detect changes.
    Str   initial_text_;
    Str   initial_font_name_;
    float initial_spacing_;

    bool ValidateText_(const Str &text);
    void ChangeFont_(const Str &font_name);
    void ChangeSpacing_(float spacing);
    void UpdateButton_();

    friend class Parser::Registry;
};
