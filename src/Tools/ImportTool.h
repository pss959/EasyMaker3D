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

    virtual bool CanAttach(const Selection &sel) const override;
    virtual Str GetPanelName() const override {
        return "ImportToolPanel";
    }
    virtual void InitPanel() override;
    virtual void PanelChanged(const Str &key,
                              ToolPanel::InteractionType type) override;
  private:
    /// Last path imported from, if any.
    FilePath last_import_path_;

    friend class Parser::Registry;
};
