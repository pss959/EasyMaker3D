#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"
#include "Util/Notifier.h"
#include "Widgets/CompositeWidget.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(DiscWidget);
DECL_SHARED_PTR(WheelWidget);

/// A WheelWidget wraps a DiscWidget used to specify a continuous rotation,
/// such as for a twist or bend. The widget contains the following named
/// sub-widget:
///   - "Rotator": A DiscWidget allowing the Wheel to be rotated.
///
/// The GetRotationChanged() Notifier can be used to track changes.
///
/// \ingroup Widgets
class WheelWidget : public CompositeWidget {
  public:
    /// Returns a Notifier that is invoked when the user rotates the
    /// Wheel.
    Util::Notifier<> & GetRotationChanged() { return rotation_changed_; }

    /// Sets the size of the widget to the given radius.
    void SetSize(float radius);

    /// Sets the rotation angle. This can be called at any time, including
    /// during a rotation drag to modify the rotation (to apply precision or
    /// snapping, for example). This does not invoke callbacks.
    void SetRotationAngle(const Anglef &angle);

    /// Returns the current rotation angle. This can be called at any time,
    /// including during a rotation drag.
    Anglef GetRotationAngle() const;

  protected:
    WheelWidget() {}

    virtual void CreationDone() override;

  private:
    /// Notifies when the widget is rotation.
    Util::Notifier<> rotation_changed_;

    /// DiscWidget used to rotate the Wheel.
    DiscWidgetPtr    rotator_;

    friend class Parser::Registry;
};
