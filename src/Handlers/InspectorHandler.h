#pragma once

#include <memory>

#include "Handlers/Handler.h"
#include "Items/Inspector.h"

/// The InspectorHandler class manages interaction for the Inspector object.
class InspectorHandler : public Handler {
  public:
    /// Sets the Inspector to handle events for. When the Inspector is
    /// disabled, this handler does nothing.
    void SetInspector(const InspectorPtr &inspector);

    virtual bool HandleEvent(const Event &event) override;

    /// Redefines this to return false if there is no Inspector or it is not
    /// enabled.
    virtual bool IsEnabled() const override;

  private:
    InspectorPtr inspector_;
};

typedef std::shared_ptr<InspectorHandler> InspectorHandlerPtr;
