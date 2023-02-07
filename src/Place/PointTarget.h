#pragma once

#include <ion/math/vectorutils.h>

#include "Base/Memory.h"
#include "Math/Curves.h"
#include "Math/Types.h"
#include "Parser/Object.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(PointTarget);

/// PointTarget represents the point-based target for interactive snapping
/// operations. It contains a position (for snapping translation or layout), a
/// direction (for snapping orientation), and several radial layout fields.
///
/// \ingroup Place
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

    /// Sets the arc for radial layout.
    void SetArc(const CircleArc &arc) { arc_ = arc; }

    /// Returns the arc for radial layout.
    CircleArc GetArc() const { return arc_; }

    /// Copies values from another instance.
    void CopyFrom(const PointTarget &from) { CopyContentsFrom(from, false); }

  protected:
    PointTarget() {}

    virtual void AddFields() override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Point3f>   position_;
    Parser::TField<Vector3f>  direction_;
    Parser::TField<float>     radius_;
    Parser::TField<CircleArc> arc_;
    ///@}

    friend class Parser::Registry;
};
