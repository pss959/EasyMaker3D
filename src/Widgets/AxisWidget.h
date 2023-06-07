#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"
#include "Util/Notifier.h"
#include "Widgets/CompositeWidget.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(AxisWidget);
DECL_SHARED_PTR(Slider2DWidget);
DECL_SHARED_PTR(SphereWidget);

/// An AxisWidget takes the form of an arrow used to specify a 3D axis. The
/// AxisWidget supports 3D rotation of the axis and optionally translation in
/// the plane perpendicular to the axis.
///
/// The widget contains the following named sub-widgets:
///   - "Rotator":  A SphereWidget allowing the axis direction to be rotated.
///   - "Translator": A Slider2DWidget allowing the axis to be translated in
///      the plane perpendicular to the axis direction (if enabled).
///
/// The GetAxisChanged() Notifier can be used to track changes to the axis
/// direction and position.
///
/// \ingroup Widgets
class AxisWidget : public CompositeWidget {
  public:
    /// Returns a Notifier that is invoked when the user drags the handle at
    /// either end of the arrow to change the rotation or the shaft to change
    /// the translation. It is passed a flag that is true for rotation and
    /// false for translation.
    Util::Notifier<bool> & GetAxisChanged() { return axis_changed_; }

    /// Sets whether translation is enabled for the AxisWidget.
    void SetTranslationEnabled(bool enabled);

    /// Sets the axis direction for the widget. The initial direction is the +Y
    /// axis.
    void SetDirection(const Vector3f &direction);

    /// Sets the position for the widget. The initial position is the origin.
    void SetPosition(const Point3f &position);

    /// Returns the current axis direction. This can be called at any time,
    /// including during a drag.
    Vector3f GetDirection() const;

    /// Returns the current axis position. This can be called at any time,
    /// including during a drag.
    Point3f GetPosition() const;

    /// Sets the size of the widget to the given radius.
    void SetSize(float radius);

    /// Sets the allowable range for the axis translation.
    void SetTranslationRange(const Range2f &range);

  protected:
    AxisWidget() {}

    virtual void CreationDone() override;

    virtual void SubWidgetActivated(const std::string &name,
                                    bool is_activation) override;

  private:
    /// Notifies when any part of the AxisWidget is dragged.
    Util::Notifier<bool> axis_changed_;

    /// Interactive SphereWidget used to rotate the axis.
    SphereWidgetPtr      rotator_;

    /// Slider2DWidget for translating the axis. This is under the rotator_, so
    /// it just always translates in X and Z.
    Slider2DWidgetPtr    translator_;

    /// Node used to translate the #rotator_ parts during translation.
    SG::NodePtr          rotator_parts_;

    /// Conical end part of the arrow.
    SG::NodePtr          cone_;

    /// Box at the base of the arrow.
    SG::NodePtr          base_;

    /// Activation/deactivation callback.
    void Activate_(bool is_activation);

    /// Invoked when the axis rotation or translation changed.
    void AxisChanged_(bool is_rotation);

    friend class Parser::Registry;
};
