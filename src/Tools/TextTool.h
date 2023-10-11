#pragma once

#include "Commands/ChangeTextCommand.h"
#include "Tools/PanelTool.h"

/// TextTool is a specialized Tool that is used to edit a TextModel. It is
/// derived from PanelTool because it uses a TextToolPanel to provide
/// interaction.
///
/// \ingroup Tools
class TextTool : public PanelTool {
  protected:
    TextTool() {}

    virtual bool CanAttach(const Selection &sel) const override;

    virtual Str GetPanelName() const override {
        return "TextToolPanel";
    }

    virtual void InitPanel() override;
    virtual void PanelChanged(const Str &key,
                              ToolPanel::InteractionType type) override;

  private:
    /// Command used to modify the TextModel.
    ChangeTextCommandPtr command_;

    friend class Parser::Registry;
};
