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
    /// Sets the initial Text to edit. This initializes the Panel for editing.
    void SetTextString(const std::string &text);

    /// Returns the current text after possible editing.
    std::string GetTextString() const;

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
    std::string      original_text_;

    bool ValidateText_(const std::string &text);
    void ChangeFont_(const std::string &font_name);
    void ChangeSpacing_(float spacing);

    friend class Parser::Registry;
};
