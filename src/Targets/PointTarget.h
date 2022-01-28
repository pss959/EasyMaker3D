#pragma once

#include <memory>

#include "Math/Types.h"
#include "Parser/Object.h"

namespace Parser { class Registry; }

/// PointTarget represents the point-based target for interactive snapping
/// operations. It contains a position (for snapping translation or layout), a
/// direction (for snapping orientation), and several radial layout fields.
///
/// \ingroup Targets
class PointTarget : public Parser::Object {
  public:
    /// Returns the target's position in stage coordinates.
    const Point3f & GetPosition() const { return position_; }

    /// Returns the target's direction vector in stage coordinates.
    const Vector3f & GetDirection() const { return direction_; }

    /// Returns the radius of the layout circle.
    float GetRadius() const { return radius_; }

    /// Returns the angle to start radial layout from.
    const Anglef & GetStartAngle() const { return start_angle_; }

    /// Returns the signed arc angle to lay out along.
    const Anglef & GetArcAngle() const { return arc_angle_; }

    /// Copies values from another instance.
    void CopyFrom(const PointTarget &from) { CopyContentsFrom(from, false); }

  protected:
    PointTarget() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Point3f>  position_{"position",   {0, 0, 0}};
    Parser::TField<Vector3f> direction_{"direction", {0, 1, 0}};
    Parser::TField<float>    radius_{"radius", 1};
    Parser::TField<Anglef>   start_angle_{"start_angle",
                                          Anglef::FromRadians(0)};
    Parser::TField<Anglef>   arc_angle_{"arc_angle", Anglef::FromRadians(0)};
    ///@}

    friend class Parser::Registry;
};

typedef std::shared_ptr<PointTarget> PointTargetPtr;
