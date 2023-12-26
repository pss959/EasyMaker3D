#pragma once

#include "Handlers/Handler.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(Inspector);
DECL_SHARED_PTR(InspectorHandler);

/// The InspectorHandler class manages interaction for the Inspector object.
///
/// \ingroup Handlers
class InspectorHandler : public Handler {
  public:
    /// Sets the Inspector to handle events for. When the Inspector is
    /// disabled, this handler does nothing.
    void SetInspector(const InspectorPtr &inspector);

    virtual HandleCode HandleEvent(const Event &event) override;

    /// Redefines this to return false if there is no Inspector or it is not
    /// enabled.
    virtual bool IsEnabled() const override;

  private:
    InspectorPtr inspector_;
};
