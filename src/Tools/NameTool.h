#pragma once

#include <memory>

#include "Commands/ChangeNameCommand.h"
#include "Tools/PanelTool.h"

/// NameTool is a general Tool that is used to edit the name of a Model.  It is
/// derived from PanelTool because it uses a NameToolPanel to provide
/// interaction.
///
/// \ingroup Tools
class NameTool : public PanelTool {
  protected:
    NameTool() {}

    virtual bool CanAttach(const Selection &sel) const override;

    virtual std::string GetPanelName() const override {
        return "NameToolPanel";
    }

    virtual void InitPanel() override;
    virtual void PanelChanged(const std::string &key,
                              ToolPanel::InteractionType type) override;

  private:
    /// Command used to modify the NameedModel data.
    ChangeNameCommandPtr command_;

    friend class Parser::Registry;
};
