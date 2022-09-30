#pragma once

#include "Base/Memory.h"
#include "Math/Curves.h"
#include "Math/Types.h"
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

    /// Sets the layout arc.
    void SetArc(const CircleArc &arc);

    /// Returns the current arc.
    const CircleArc & GetArc() const { return arc_; }

    /// Sets a matrix converting from object to world coordinates and the
    /// rotation applied to the widget. These are used to orient text.
    void SetTextOrientation(const Matrix4f &owm, const Rotationf &rot) {
        text_matrix_   = owm;
        text_rotation_ = rot;
    }

    /// Resets all values to their defaults.
    void Reset();

    /// Redefines this to set up colors.
    virtual void PostSetUpIon() override;

  protected:
    RadialLayoutWidget() {}

    virtual void CreationDone() override;

  private:
    /// Notifies when the widget is dragged in any way.
    Util::Notifier<> changed_;

    float            radius_ = 1;   ///< Outer radius of ring.
    CircleArc        arc_;          ///< Arc defined by spokes.
    Anglef           end_angle_;    ///< Saves the current end angle.

    /// Radius at the start of a drag.
    float            start_radius_;

    /// \name Interactive Parts.
    ///@{
    DiscWidgetPtr    ring_;         ///< Torus to resize radius.
    DiscWidgetPtr    start_spoke_;  ///< Spoke to change start angle.
    DiscWidgetPtr    end_spoke_;    ///< Spoke to change arc angle.
    ///@}

    /// \name Feedback Parts.
    ///@{
    SG::NodePtr      arc_line_;          ///< Shows layout arc as a line.
    SG::TextNodePtr  radius_text_;       ///< Shows current radius.
    SG::TextNodePtr  start_angle_text_;  ///< Shows start angle
    SG::TextNodePtr  end_angle_text_;    ///< Shows end angle
    SG::TextNodePtr  arc_angle_text_;    ///< Shows arc angle
    ///@}

    /// \name Other Parts.
    ///@{
    SG::NodePtr      layout_;       ///< Root of layout UI.
    SG::NodePtr      spoke_geom_;   ///< Spoke geometry for sizing.
    ///@}

    /// Saves the angle at the start of a spoke rotation.
    Anglef           start_rot_angle_;

    Matrix4f  text_matrix_ = Matrix4f::Identity();
    Rotationf text_rotation_;

    /// Sets the colors for various parts.
    void SetColors_();

    // Sub-widget callbacks.
    void RadiusActivated_(bool is_activation);
    void RadiusChanged_(float new_radius, float precision);
    void SpokeActivated_(bool is_activation, bool is_start);
    void SpokeChanged_(const Anglef &angle, bool is_start, float precision);

    /// This is called when the end spoke is rotated. If the end spoke crossed
    /// over the start spoke, this returns the new signed angle between them.
    /// Otherwise it returns a zero angle.
    Anglef GetCrossoverAngle_(const Anglef &new_end_angle);

    /// Updates the ring to reflect the current radius.
    void UpdateRing_();

    /// Updates the spokes to reflect the current radius and angles.
    void UpdateSpokes_();

    /// Updates the arc showing the current subtended angle and the angle text.
    void UpdateArc_();

    /// Convenience to build a Rotationf from an angle.
    static Rotationf BuildRotation_(const Anglef &angle) {
        return Rotationf::FromAxisAndAngle(Vector3f::AxisY(), angle);
    }

    static std::string GetAngleText_(const Anglef &angle);

    friend class Parser::Registry;
};
