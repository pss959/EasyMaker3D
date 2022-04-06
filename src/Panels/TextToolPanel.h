#pragma once

#include <string>

#include "Memory.h"
#include "ToolPanel.h"

DECL_SHARED_PTR(DropdownPane);
DECL_SHARED_PTR(SliderPane);
DECL_SHARED_PTR(TextToolPanel);
DECL_SHARED_PTR(TextInputPane);
DECL_SHARED_PTR(TextPane);

namespace Parser { class Registry; }

/// TextToolPanel is a derived ToolPanel class thatis used by the TextTool for
/// interactive editing of a TextModel.
///
/// ReportChange keys: "XXXX" (immediate).
///
/// \ingroup Panels
class TextToolPanel : public ToolPanel {
  public:
    /// Initializes the text string, font name, and character spacing.
    void SetValues(const std::string &text, const std::string &font_name,
                   float char_spacing);

    /// Returns the current text string after possible editing.
    std::string GetTextString() const;

    /// Returns the current font name after possible editing.
    const std::string & GetFontName() const;

    /// Returns the character spacing after possible editing.
    float GetCharSpacing() const;

  protected:
    TextToolPanel() {}

    virtual void CreationDone() override;
    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

  private:
    TextInputPanePtr text_pane_;
    DropdownPanePtr  font_pane_;
    SliderPanePtr    spacing_pane_;
    TextPanePtr      display_pane_;
    TextPanePtr      message_pane_;

    bool ValidateText_(const std::string &text);
    void ChangeFont_(const std::string &font_name);
    void ChangeSpacing_(float spacing);

    friend class Parser::Registry;
};
