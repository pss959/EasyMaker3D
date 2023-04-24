#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"
#include "Parser/Field.h"
#include "Widgets/SliderWidgetBase.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(Slider2DWidget);

/// Slider2DWidget is a derived SliderWidgetBase that provides interaction
/// within a constrained rectangle in one of the principal planes.
///
/// \ingroup Widgets
class Slider2DWidget : public SliderWidgetBase<Vector2f> {
  public:
    /// Defines the principal plane the widget operates in.
    enum class PrincipalPlane {
        kXY,   ///< XY plane (default).
        kXZ,   ///< XZ plane.
        kYZ,   ///< YZ plane.
    };

    /// Returns the principal plane the Slider2DWidget operates in.
    PrincipalPlane GetPrincipalPlane() const { return principal_plane_; }

    virtual Vector2f GetInterpolated() const override;
    virtual Vector2f GetRayValue(const Ray &local_ray) override;
    virtual Vector2f GetControllerMotion(const Point3f &p0,
                                         const Point3f &p1) override;
    virtual void UpdatePosition() override;

  protected:
    Slider2DWidget() {}

    virtual void AddFields() override;

    /// Redefines this to set up the intersection plane based on the starting
    /// intersection point.
    virtual void PrepareForDrag(const DragInfo &info) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::EnumField<PrincipalPlane> principal_plane_;
    ///@}

    /// Plane used for intersections.
    Plane   intersection_plane_;

    /// Converts a Vector2f to a Vector3f based on the PrincipalPlane.
    Vector3f ToVector3f_(const Vector2f &v2) const;

    /// Returns the dimension perpendicular to the PrincipalPlane.
    int GetDimension_() const;

    friend class Parser::Registry;
};
