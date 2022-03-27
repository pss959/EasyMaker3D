#pragma once

#include "Commands/ChangeBevelCommand.h"
#include "Tools/PanelTool.h"

/// BevelTool is a specialized Tool that is used to edit the Bevel data for
/// selected BeveledModel instances. It is derived from PanelTool because it
/// uses a BevelToolPanel to provide interaction.
///
/// \ingroup Tools
class BevelTool : public PanelTool {
  protected:
    BevelTool() {}

    virtual bool IsSpecialized() const override { return true; }
    virtual bool CanAttach(const Selection &sel) const override;

    virtual std::string GetPanelName() const override {
        return "BevelToolPanel";
    }

    virtual void InitPanel() override;
    virtual void PanelChanged(const std::string &key,
                              ToolPanel::InteractionType type) override;

  private:
    /// Command used to modify the BeveledModel data.
    ChangeBevelCommandPtr command_;

    friend class Parser::Registry;
};
