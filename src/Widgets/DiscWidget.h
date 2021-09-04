#pragma once

#include "Math/Types.h"
#include "Widgets/IDraggableWidget.h"
#include "Widgets/Widget.h"

//! DiscWidget implements both rotation and scaling on an infinite disc
//! centered on the origin in the XZ-plane (rotating about the Y axis). It
//! modifies its transformations based on interaction.
//!
//! \ingroup Widgets
class DiscWidget : public Widget, public IDraggableWidget {
  public:
    //! Returns a Notifier that is invoked when the user drags the widget to
    //! cause rotation. It is passed the widget and the change in rotation
    //! angle around the axis from the start of the drag.
    Util::Notifier<Widget&, const Anglef &> & GetRotationChanged() {
        return rotation_changed_;
    }

    //! Returns a Notifier that is invoked when the user drags the widget to
    //! cause a change in scale (when enabled). It is passed the widget and the
    //! change in scale as a signed value from the start of the drag.
    Util::Notifier<Widget&, float> & GetScaleChanged() {
        return scale_changed_;
    }

    //! Sets a flag indicating whether scaling is allowed. The default is true.
    void SetScalingAllowed(bool allowed) { is_scaling_allowed_ = allowed; }
    //! Returns a flag indicating whether scaling is allowed.
    bool IsScalingAllowed() const        { return is_scaling_allowed_; }

    virtual void StartDrag(const DragInfo &info) override;
    virtual void Drag(const DragInfo &info) override;
    virtual void EndDrag() override;

  private:
    //! Notifies when the widget is rotated.
    Util::Notifier<Widget&, const Anglef &> rotation_changed_;

    //! Notifies when the widget is scaled.
    Util::Notifier<Widget&, float> scale_changed_;

    //! True if interactive scaling is allowed.
    bool is_scaling_allowed_ = true;
};
