#pragma once

#include "Tools/PanelTool.h"

/// CSGTool is a derived PanelTool that is used to modify the CSGOperation
/// of one or more to selected CSGModels.
//
// \ingroup Tools
class CSGTool : public PanelTool {
  protected:
    CSGTool() {}

    virtual bool IsSpecialized() const override { return true; }
    virtual bool CanAttach(const Selection &sel) const override;

    virtual std::string GetPanelName() const override {
        return "CSGToolPanel";
    }

    virtual void InitPanel() override;
    virtual void PanelChanged(const std::string &key,
                              ToolPanel::InteractionType type) override;

  private:
    friend class Parser::Registry;
};
