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
    /// Returns a Notifier that is invoked when the user drags while the
    /// GenericWidget is the active Widget. It is passed a pointer to DragInfo
    /// for the drag, and a flag indicating whether this is the start of the
    /// drag. The end of the drag is indicated by a null DragInfo pointer.
    Util::Notifier<const DragInfo *, bool> & GetDragged() { return dragged_; }

    /// Returns a Notifier that is invoked when an Event causes the
    /// GenericWidget to become or remain hovered. The hovered point on the
    /// Widget is passed in.
    Util::Notifier<const Point3f &> & GetHovered() { return hovered_; }

    virtual void StartDrag(const DragInfo &info) override;
    virtual void ContinueDrag(const DragInfo &info) override;
    virtual void EndDrag() override;

    /// Redefines this to invoke the GetHovered() Notifier.
    virtual void UpdateHoverPoint(const Point3f &point) override;

  protected:
    GenericWidget() {}

  private:
    Util::Notifier<const DragInfo *, bool> dragged_;
    Util::Notifier<const Point3f &>        hovered_;

    friend class Parser::Registry;
};
