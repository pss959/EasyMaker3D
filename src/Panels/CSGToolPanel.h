#pragma once

#include "Enums/CSGOperation.h"
#include "Panels/ToolPanel.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(CSGToolPanel);

namespace Parser { class Registry; }

/// CSGToolPanel is a derived ToolPanel class that is used by the CSGTool to
/// interactively change the CSG operation.
///
/// ReportChange() keys: "Operation" (immediate).
///
/// \ingroup Panels
class CSGToolPanel : public ToolPanel {
  public:
    /// Sets the initial CSG operation to display. This initializes the Panel
    /// for editing.
    void SetOperation(CSGOperation operation);

    /// Returns the current CSG operation after possible changes.
    CSGOperation GetOperation() const { return operation_; }

  protected:
    CSGToolPanel() {}

    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

  private:
    /// Currently selected CSG operation.
    CSGOperation operation_ = CSGOperation::kUnion;

    friend class Parser::Registry;
};
