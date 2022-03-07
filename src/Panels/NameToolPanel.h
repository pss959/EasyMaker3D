#pragma once

#include <memory>

#include "Panes/TextInputPane.h"
#include "Panes/TextPane.h"
#include "ToolPanel.h"

namespace Parser { class Registry; }

/// NameToolPanel is a derived ToolPanel class thatis used by the NameTool
/// for interactive editing of a Model's name.
///
/// ReportChange keys: none.
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

  private:
    TextInputPanePtr input_pane_;
    TextPanePtr      message_pane_;

    friend class Parser::Registry;
};

typedef std::shared_ptr<NameToolPanel> NameToolPanelPtr;
