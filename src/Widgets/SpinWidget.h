#pragma once

#include "Base/Memory.h"
#include "Math/Spin.h"
#include "Math/Types.h"
#include "Util/Notifier.h"
#include "Widgets/CompositeWidget.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(AxisWidget);
DECL_SHARED_PTR(DiscWidget);
DECL_SHARED_PTR(Slider1DWidget);
DECL_SHARED_PTR(SpinWidget);

/// A SpinWidget is used to specify a continuous rotation about an axis defined
/// as a Spin, such as for a twist or bend. The widget contains the following
/// named sub-widgets:
///   - "Axis": An AxisWidget allowing the rotation axis to be rotated and
///     translated.
///   - "Ring": A DiscWidget providing rotation.
///
/// The GetSpinChanged() Notifier can be used to track changes.
///
/// \ingroup Widgets
class SpinWidget : public CompositeWidget {
  public:
    /// Defines which type of change occurred in the GetSpinChanged()
    /// Notifier.
    enum class ChangeType {
        kAxis,    ///< Axis direction was rotated.
        kCenter,  ///< Axis center was translated.
        kAngle,   ///< Spin was rotated to change the angle.
        kOffset,  ///< Offset slider changed the spin offset.
    };

    /// Returns a Notifier that is invoked when the user changes the current
    /// Spin. It is passed a ChangeType.
    Util::Notifier<ChangeType> & GetSpinChanged() { return spin_changed_; }

    /// Sets the Spin for the widget. The initial Spin is around the +Y axis by
    /// 0 degrees.
    void SetSpin(const Spin &spin);

    /// Returns the current spin. This can be called at any time, including
    /// during a drag.
    const Spin & GetSpin() const { return spin_; }

    /// Sets the size of the widget to the given radius.
    void SetSize(float radius);

  protected:
    SpinWidget() {}

    virtual void CreationDone() override;

  private:
    /// Notifies when the user has used the widget to change the Spin.
    Util::Notifier<ChangeType> spin_changed_;

    /// Current Spin.
    Spin                       spin_;

    /// AxisWidget used to position and rotate the axis.
    AxisWidgetPtr              axis_;

    /// DiscWidget with a ring used to change the angle.
    DiscWidgetPtr              ring_;

    /// Slider1DWidget used to change the offset.
    Slider1DWidgetPtr          offset_;

    /// Node used to rotate and translate the ring to match the spin axis.
    SG::NodePtr                ring_transform_;

    /// Updates #spin_ to match the current spin. Called when something
    /// changes.
    void UpdateSpin_(ChangeType type);

    friend class Parser::Registry;
};
