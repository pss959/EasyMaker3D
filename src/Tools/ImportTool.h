#pragma once

#include "Tools/PanelTool.h"
#include "Util/FilePath.h"

/// ImportTool is a specialized Tool that is used to edit the file path of an
/// ImportedModel.  It uses an ImportToolPanel to provide interaction.
///
/// \ingroup Tools
class ImportTool : public PanelTool {
  protected:
    ImportTool() {}

    virtual bool IsSpecialized() const { return true; }
    virtual bool CanAttach(const Selection &sel) const override;
    virtual std::string GetPanelName() const override {
        return "ImportToolPanel";
    }
    virtual void InitPanel() override;
    virtual void PanelChanged(const std::string &key,
                              ToolPanel::InteractionType type) override;
  private:
    /// Last path imported from, if any.
    FilePath last_import_path_;

    friend class Parser::Registry;
};
