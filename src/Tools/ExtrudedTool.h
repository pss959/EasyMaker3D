#pragma once

#include "Commands/ChangeExtrudedCommand.h"
#include "Tools/PanelTool.h"

/// ExtrudedTool is a specialized Tool that is used to edit the profile for
/// selected ExtrudedModel instances. It is derived from PanelTool because it
/// uses a ExtrudedToolPanel to provide interaction.
///
/// \ingroup Tools
class ExtrudedTool : public PanelTool {
  protected:
    ExtrudedTool() {}

    virtual bool CanAttach(const Selection &sel) const override;
    virtual void Attach() override;
    virtual void Detach() override;

    virtual Str GetPanelName() const override {
        return "ExtrudedToolPanel";
    }

    virtual void InitPanel() override;
    virtual void PanelChanged(const Str &key,
                              ToolPanel::InteractionType type) override;

  private:
    /// Command used to modify the ExtrudedModel data.
    ChangeExtrudedCommandPtr command_;

    /// Set to true if a point was dragged.
    bool                     point_dragged_ = false;

    // Updates the point precision in the ExtrudedToolPanel when the tool is
    // attached or when the precision changes.
    void UpdatePrecision_();

    friend class Parser::Registry;
};
