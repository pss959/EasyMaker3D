#pragma once

#include <string>

#include "Base/Memory.h"
#include "Panels/ToolPanel.h"

DECL_SHARED_PTR(NameToolPanel);
DECL_SHARED_PTR(TextInputPane);
DECL_SHARED_PTR(TextPane);

namespace Parser { class Registry; }

/// NameToolPanel is a derived ToolPanel class that is used by the NameTool
/// for interactive editing of a Model's name.
///
/// ReportChange keys: "Name" (immediate).
///
/// \ingroup Panels
class NameToolPanel : public ToolPanel {
  public:
    /// Sets the initial Name to edit. This initializes the Panel for editing.
    void SetName(const std::string &name);

    /// Returns the current name after possible editing.
    std::string GetName() const;

  protected:
    NameToolPanel() {}

    virtual void CreationDone() override;
    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

  private:
    TextInputPanePtr input_pane_;
    TextPanePtr      message_pane_;
    std::string      original_name_;

    bool ValidateName_(const std::string &name);

    friend class Parser::Registry;
};
