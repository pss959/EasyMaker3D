#pragma once

#include "Commands/ChangeRevSurfCommand.h"
#include "Tools/PanelTool.h"

/// RevSurfTool is a specialized Tool that is used to edit the profile and
/// sweep angle data for selected RevSurfModel instances. It is derived from
/// PanelTool because it uses a RevSurfToolPanel to provide interaction.
///
/// \ingroup Tools
class RevSurfTool : public PanelTool {
  protected:
    RevSurfTool() {}

    virtual bool IsSpecialized() const override { return true; }
    virtual bool CanAttach(const Selection &sel) const override;
    virtual void Attach() override;
    virtual void Detach() override;

    virtual std::string GetPanelName() const override {
        return "RevSurfToolPanel";
    }

    virtual void InitPanel() override;
    virtual void PanelChanged(const std::string &key,
                              ToolPanel::InteractionType type) override;

  private:
    /// Command used to modify the RevSurfModel data.
    ChangeRevSurfCommandPtr command_;

    /// Set to true if a point was dragged.
    bool                    point_dragged_ = false;

    // Updates the point precision in the RevSurfToolPanel when the tool is
    // attached or when the precision changes.
    void UpdatePrecision_();

    friend class Parser::Registry;
};
