#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"
#include "Widgets/DraggableWidget.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(GenericWidget);

/// A GenericWidget does nothing but pass click and drag events to callers via
/// the GetClicked() and GetDragged() Notifiers. It can have any geometry as
/// children.
///
/// \ingroup Widgets
class GenericWidget : public DraggableWidget {
  public:
    virtual void StartDrag(const DragInfo &info) override;
    virtual void EndDrag() override;

  protected:
    GenericWidget() {}

  private:
    friend class Parser::Registry;
};
