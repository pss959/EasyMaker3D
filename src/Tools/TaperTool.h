#pragma once

#include "Commands/ChangeTaperCommand.h"
#include "Math/Bevel.h"
#include "Tools/PanelTool.h"

/// TaperTool is a specialized Tool that is used to edit the Taper data for
/// selected TaperedModel instances. It is derived from PanelTool because it
/// uses a TaperToolPanel to provide interaction.
///
/// \ingroup Tools
class TaperTool : public PanelTool {
  protected:
    TaperTool() {}

    virtual bool IsSpecialized() const override { return true; }
    virtual bool CanAttach(const Selection &sel) const override;

    virtual std::string GetPanelName() const override {
        return "TaperToolPanel";
    }

    virtual void InitPanel() override;
    virtual void PanelChanged(const std::string &key,
                              ToolPanel::InteractionType type) override;

  private:
    /// Command used to modify the TaperedModel data.
    ChangeTaperCommandPtr command_;

    /// Taper at the start of a drag.
    Bevel start_taper_;

    friend class Parser::Registry;
};
