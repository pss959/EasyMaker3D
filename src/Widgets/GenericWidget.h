#pragma once

#include <memory>

#include "Widgets/DraggableWidget.h"

namespace Parser { class Registry; }

/// A GenericWidget does nothing but pass click and drag events to callers via
/// the GetClicked() and GetDragged() Notifiers. It can have any geometry as
/// children.
///
/// \ingroup Widgets
class GenericWidget : public DraggableWidget {
  public:
    /// Returns a Notifier that is invoked when the user drags anywhere on the
    /// GenericWidget's geometry. It is passed a pointer to DragInfo for the
    /// drag, and a flag indicating whether this is the start of the drag. The
    /// end of the drag is indicated by a null DragInfo pointer.
    Util::Notifier<const DragInfo *, bool> & GetDragged() { return dragged_; }

    virtual void StartDrag(const DragInfo &info) override;
    virtual void ContinueDrag(const DragInfo &info) override;
    virtual void EndDrag() override;

  protected:
    GenericWidget() {}

  private:
    Util::Notifier<const DragInfo *, bool> dragged_;

    friend class Parser::Registry;
};

typedef std::shared_ptr<GenericWidget> GenericWidgetPtr;
