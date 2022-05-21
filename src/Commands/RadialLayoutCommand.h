#pragma once

#include "Base/Memory.h"
#include "Commands/MultiModelCommand.h"
#include "Math/Types.h"

namespace Parser { class Registry; }

class PointTarget;
DECL_SHARED_PTR(RadialLayoutCommand);

/// RadialLayoutCommand is used to lay out the currently selected Models
/// in a radial pattern in a plane.
///
/// \ingroup Commands
class RadialLayoutCommand : public MultiModelCommand {
  public:
    virtual std::string GetDescription() const override;

    /// Sets the fields in the command from a PointTarget instance.
    void SetFromTarget(const PointTarget &target);

    // XXXX
    //void SetLayout(const Point3f &center, const Vector3f &normal, float radius,
    //const Anglef &start_angle, const Anglef &arc_angle);

    /// Returns the center point.
    const Point3f & GetCenter() const { return center_; }

    /// Returns the normal to the layout plane.
    const Vector3f & GetNormal() const { return normal_; }

    /// Returns the layout radius.
    float GetRadius() const { return radius_; }

    /// Returns the start angle in degrees.
    const Anglef & GetStartAngle() const { return start_angle_; }

    /// Returns the arc angle in degrees.
    const Anglef & GetArcAngle() const { return arc_angle_; }

  protected:
    RadialLayoutCommand() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Point3f>  center_{"center", {0, 0, 0}};
    Parser::TField<Vector3f> normal_{"normal", {0, 1, 0}};
    Parser::TField<float>    radius_{"radius", 1};
    Parser::TField<Anglef>   start_angle_{"start_angle", Anglef()};
    Parser::TField<Anglef>   arc_angle_{"arc_angle", Anglef::FromDegrees(-360)};
    ///@}

    friend class Parser::Registry;
};
