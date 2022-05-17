#pragma once

#include <ion/math/vectorutils.h>

#include "Math/Types.h"
#include "Memory.h"
#include "Parser/Object.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(PointTarget);

/// PointTarget represents the point-based target for interactive snapping
/// operations. It contains a position (for snapping translation or layout), a
/// direction (for snapping orientation), and several radial layout fields.
///
/// \ingroup Targets
class PointTarget : public Parser::Object {
  public:
    /// Sets the target's position in stage coordinates.
    void SetPosition(const Point3f &pos) { position_ = pos; }

    /// Returns the target's position in stage coordinates.
    const Point3f & GetPosition() const { return position_; }

    /// Sets the target's direction in stage coordinates.
    void SetDirection(const Vector3f &dir) {
        direction_ = ion::math::Normalized(dir);
    }

    /// Returns the target's unit direction vector in stage coordinates.
    const Vector3f & GetDirection() const { return direction_; }

    /// Sets the radius of the layout circle.
    void SetRadius(float rad) { radius_ = rad; }

    /// Returns the radius of the layout circle.
    float GetRadius() const { return radius_; }

    /// Sets the angle to start radial layout from.
    void SetStartAngle(const Anglef &angle) { start_angle_ = angle; }

    /// Returns the angle to start radial layout from.
    const Anglef & GetStartAngle() const { return start_angle_; }

    /// Sets the signed arc angle to lay out along.
    void SetArcAngle(const Anglef &angle) { arc_angle_ = angle; }

    /// Returns the signed arc angle to lay out along.
    const Anglef & GetArcAngle() const { return arc_angle_; }

    /// Copies values from another instance.
    void CopyFrom(const PointTarget &from) { CopyContentsFrom(from, false); }

  protected:
    PointTarget() {}

    virtual void AddFields() override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Point3f>  position_{"position",   {0, 0, 0}};
    Parser::TField<Vector3f> direction_{"direction", {0, 1, 0}};
    Parser::TField<float>    radius_{"radius", 1};
    Parser::TField<Anglef>   start_angle_{"start_angle",
                                          Anglef::FromDegrees(0)};
    Parser::TField<Anglef>   arc_angle_{"arc_angle", Anglef::FromDegrees(-360)};
    ///@}

    friend class Parser::Registry;
};
