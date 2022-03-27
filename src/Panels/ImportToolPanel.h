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

  private:

    friend class Parser::Registry;
};
