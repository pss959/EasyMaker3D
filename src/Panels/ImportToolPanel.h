#pragma once

#include <string>

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
    /// Displays the given import error and waits for a response.
    void DisplayImportError(const std::string &message);

  protected:
    ImportToolPanel() {}

    /// Redefines this to not close the panel.
    virtual void ProcessResult(const std::string &result) override;

  private:
    friend class Parser::Registry;
};
