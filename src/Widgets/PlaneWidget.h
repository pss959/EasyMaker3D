#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"
#include "Util/Notifier.h"
#include "Widgets/CompositeWidget.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(PlaneWidget);
DECL_SHARED_PTR(Slider1DWidget);
DECL_SHARED_PTR(SphereWidget);

/// A PlaneWidget is used to specify the orientation and position of a plane in
/// 3D. The widget contains the following parts:
///   - A square showing the plane that is also a SphereWidget used to rotate
///     it.
///   - An arrow showing the plane normal that is also a Slider1DWidget used to
///     translate it along the normal direction.
///
/// The GetChanged() Notifier can be used to track changes.
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
    Plane GetPlane() const;

    /// Sets the size of the widget to the given radius.
    void SetSize(float radius);

    /// Sets the allowable range for the plane translation.
    void SetTranslationRange(const Range1f &range);

    /// Returns the SphereWidget used to orient the plane.
    SphereWidgetPtr GetRotator() const { return rotator_; }

    /// Returns the Slider1DWidget used to translate the plane.
    Slider1DWidgetPtr GetTranslator() const { return translator_; }

  protected:
    PlaneWidget() {}

    virtual void CreationDone() override;

  private:
    /// Notifies when the widget is dragged.
    Util::Notifier<bool> plane_changed_;

    /// Current Plane.
    Plane plane_;

    /// Interactive SphereWidget used to rotate the plane.
    SphereWidgetPtr   rotator_;

    /// Slider1DWidget with an arrow for translating the plane.
    Slider1DWidgetPtr translator_;

    /// Cylindrical shaft part of the arrow. This is scaled in Y based on the
    /// size passed to SetSize().
    SG::NodePtr       arrow_shaft_;

    /// Conical end part of the arrow. This is translated in Y to stay at the
    /// end of the arrow_shaft.
    SG::NodePtr       arrow_cone_;

    /// Invoked when either sub-widget is activated or deactivated.
    void Activate_(bool is_activation);

    /// Invoked when the plane rotation changed.
    void RotationChanged_();

    /// Invoked when the plane translation changed.
    void TranslationChanged_();

    friend class Parser::Registry;
};
