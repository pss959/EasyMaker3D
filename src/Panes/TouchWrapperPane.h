#pragma once

#include "Base/Memory.h"
#include "Panes/BoxPane.h"
#include "Panes/IPaneInteractor.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(TouchWrapperPane);

/// TouchWrapperPane is a derived BoxPane that implements the IPaneInteractor
/// interface so that touch interaction works with non-Pane Widgets contained
/// in it.
///
/// \ingroup Panes
class TouchWrapperPane : public BoxPane, public IPaneInteractor {
  public:
    // IPaneInteractor interface.
    virtual IPaneInteractor * GetInteractor() override { return this; }
    virtual bool CanFocus() const override;

  protected:
    TouchWrapperPane() {}

  private:
    friend class Parser::Registry;
};
