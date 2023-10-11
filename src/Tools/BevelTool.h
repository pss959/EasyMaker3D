#pragma once

#include "Commands/ChangeBevelCommand.h"
#include "Math/Bevel.h"
#include "Tools/PanelTool.h"

/// BevelTool is a specialized Tool that is used to edit the Bevel data for
/// selected BeveledModel instances. It is derived from PanelTool because it
/// uses a BevelToolPanel to provide interaction.
///
/// \ingroup Tools
class BevelTool : public PanelTool {
  protected:
    BevelTool() {}

    virtual bool CanAttach(const Selection &sel) const override;

    virtual Str GetPanelName() const override {
        return "BevelToolPanel";
    }

    virtual void InitPanel() override;
    virtual void PanelChanged(const Str &key,
                              ToolPanel::InteractionType type) override;

  private:
    /// Command used to modify the BeveledModel data.
    ChangeBevelCommandPtr command_;

    /// Bevel at the start of a drag.
    Bevel start_bevel_;

    friend class Parser::Registry;
};
