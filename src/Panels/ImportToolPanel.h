#pragma once

#include "Memory.h"
#include "FilePanel.h"

DECL_SHARED_PTR(ImportToolPanel);

namespace Parser { class Registry; }

/// ImportToolPanel is a derived FilePanel class that is used by the ImportTool
/// to select the file to import STL data from.
///
/// ReportChange keys: "Cancel", "Accept" (both immediate).
///
/// \ingroup Panels
class ImportToolPanel : public FilePanel {
  public:

  protected:
    ImportToolPanel() {}

    /// Redefines this to not close the panel.
    virtual void ProcessResult(const std::string &result) override {
        ReportChange(result, InteractionType::kImmediate);
    }

  private:
    friend class Parser::Registry;
};
