#pragma once

#include "Math/Plane.h"
#include "Math/Types.h"
#include "Util/Memory.h"
#include "Util/Notifier.h"
#include "Widgets/CompositeWidget.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(AxisWidget);
DECL_SHARED_PTR(PlaneWidget);
DECL_SHARED_PTR(Slider1DWidget);

/// A PlaneWidget is used to specify the orientation and position of a plane in
/// 3D. The widget contains the following named sub-widgets:
///   - "Axis":  An AxisWidget allowing the plane normal to be rotated.
///   - "Plane": A Slider1DWidget connected to a square showing the plane and
///      allowing it to be translated along the normal.
///
/// The GetPlaneChanged() Notifier can be used to track changes.
///
/// \ingroup Widgets
class PlaneWidget : public CompositeWidget {
  public:
    /// Returns a Notifier that is invoked when the user drags the widget to
    /// change the plane. It is passed a flag that is true for rotation and
    /// false for translation.
    Util::Notifier<bool> & GetPlaneChanged() { return plane_changed_; }

    /// Sets the Plane for the widget. The initial Plane is the XY-plane with
    /// the normal pointing along +Z.
    void SetPlane(const Plane &plane);

    /// Returns the current plane. This can be called at any time, including
    /// during a drag.
    const Plane & GetPlane() const { return plane_; }

    /// Sets the size of the widget to the given radius.
    void SetSize(float radius);

    /// Sets the allowable range for the plane translation.
    void SetTranslationRange(const Range1f &range);

  protected:
    PlaneWidget() {}

    virtual void CreationDone() override;

  private:
    /// Notifies when the widget is dragged.
    Util::Notifier<bool> plane_changed_;

    /// Current Plane.
    Plane                plane_;

    /// AxisWidget used to rotate the plane.
    AxisWidgetPtr        rotator_;

    /// Slider1DWidget for translating the plane.
    Slider1DWidgetPtr    translator_;

    /// Rotates the Slider1DWidget.
    SG::NodePtr          plane_rotator_;

    /// Updates the #plane_ to match the current plane. Called when rotation or
    /// translation changes.
    void UpdatePlane_(bool is_rotation);

    friend class Parser::Registry;
};
