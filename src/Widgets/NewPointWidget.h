#pragma once

#include "Base/Memory.h"
#include "Widgets/DraggableWidget.h"

DECL_SHARED_PTR(NewPointWidget);
DECL_SHARED_PTR(Slider2DWidget);

/// NewPointWidget is a special DraggableWidget that is used by the ProfilePane
/// to simulate a drag to create a new Profile point. When a drag on the
/// NewPointWidget is initiated, the ProfilePane creates a new Slider2DWidget
/// for the new point and installs it in the NewPointWidget so it can delegate
/// the drag functions to it.
///
/// \ingroup Widgets
class NewPointWidget : public DraggableWidget {
  public:
    /// Sets the Slider2DWidget to delegate to. This must be called before
    /// dragging the NewPointWidget.
    void SetDelegateWidget(const Slider2DWidgetPtr &widget) {
        delegate_widget_ = widget;
    }

    virtual void StartDrag(const DragInfo &info) override;
    virtual void ContinueDrag(const DragInfo &info) override;
    virtual void EndDrag() override;

  private:
    Slider2DWidgetPtr delegate_widget_;
};
