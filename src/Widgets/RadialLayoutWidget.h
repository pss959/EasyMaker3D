#pragma once

#include "Math/Types.h"
#include "Memory.h"
#include "Util/Notifier.h"
#include "Widgets/Widget.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(DiscWidget);
DECL_SHARED_PTR(RadialLayoutWidget);
namespace SG {
DECL_SHARED_PTR(Node);
DECL_SHARED_PTR(TextNode);
}

/// RadialLayoutWidget is a specialized Widget for setting up radial layout -
/// it allows the user to define a circle with adjustable radius and optional
/// start and end angles. The widget contains the following geometry:
///   - A ring (torus) with a DiscWidget attached to it for scaling the radius.
///   - Two radial spokes with DiscWidgets attached to them for rotating the
///     start and end angles.
///
/// To avoid clutter, the spokes are hidden when the radius is small. The
/// spokes default to both be positioned at 0 degrees, meaning that the entire
/// circle is used. Dragging the start angle rotates both of the spokes
/// together, changing the starting position of the layout. Dragging the end
/// angle leaves the start angle alone and therefore changes the layout angle
/// between them, which is indicated by an arrow. Text feedback shows both
/// angles when useful.
///
/// The radius and angles are affected by the current precision.
///
/// The layout direction depends on how the angles are dragged. The default
/// layout has a start angle of 0 and an arc angle of -360 degrees (clockwise
/// rotation). Dragging the end angle spoke counterclockwise past the start
/// angle will switch to counterclockwise, and similarly for clockwise.
///
/// The widget is defined in the XZ-plane, centered on the origin. It can be
/// transformed to other orientations and positions.
///
/// The GetChanged() Notifier can be used to track changes.
///
/// \ingroup Widgets
class RadialLayoutWidget : public Widget {
  public:
    /// Returns a Notifier that is invoked when the user drags any part of the
    /// widget.
    Util::Notifier<> & GetChanged() { return changed_; }

    /// Sets the radius.
    void SetRadius(float radius);

    /// Returns the current radius.
    float GetRadius() const { return radius_; }

    /// Sets the start angle and (signed) arc angle.
    void SetAngles(const Anglef &start_angle, const Anglef &arc_angle);

    /// Returns the current start angle.
    const Anglef & GetStartAngle() const { return start_angle_; }

    /// Returns the current signed arc angle.
    const Anglef & GetArcAngle() const { return arc_angle_; }

    /// Resets all values to their defaults.
    void Reset();

  protected:
    RadialLayoutWidget() {}

    virtual void CreationDone() override;

  private:
    /// Notifies when the widget is dragged in any way.
    Util::Notifier<> changed_;

    float  radius_ = 1;
    Anglef start_angle_;
    Anglef arc_angle_{ Anglef::FromDegrees(-360) };

    /// Radius at the start of a drag.
    float start_radius_;

    // Parts.
    DiscWidgetPtr   ring_;
    SG::NodePtr     layout_;
    SG::NodePtr     spoke_;
    DiscWidgetPtr   start_spoke_;
    DiscWidgetPtr   end_spoke_;
    SG::NodePtr     arc_;
    SG::TextNodePtr radius_text_;

    // Sub-widget callbacks.
    void RadiusActivated_(bool is_activation);
    void RadiusChanged_(float new_radius, float precision);
    void SpokeActivated_(bool is_activation);
    void SpokeChanged_(const Anglef &angle, bool is_start, float precision);

    /// Updates the ring to reflect the current radius.
    void UpdateRing_();

    /// Updates the spokes to reflect the current radius and angles.
    void UpdateSpokes_();

    /// Updates the arc showing the current subtended angle.
    void UpdateArc_();

    void UpdateRadiusText_();

    friend class Parser::Registry;
};
