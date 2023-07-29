#pragma once

#include <vector>

#include "Base/Memory.h"
#include "Math/Curves.h"
#include "Math/Types.h"
#include "SG/PrimitiveShape.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(PolyLine);

/// PolyLine is a derived PrimitiveShape that represents a set of connected 3D
/// line segments. It does not define normals or texture coordinates, so use a
/// shader that does not require them. It does not respond to intersection
/// queries.
///
/// \ingroup SG
class PolyLine : public PrimitiveShape {
  public:
    const std::vector<Point3f> & GetPoints() const { return points_; }

    void SetPoints(const std::vector<Point3f> &points);

    /// Sets points to form an arc around the origin in the Z=0 plane. If the
    /// arc angle is not large enough, this clears the points.
    void SetArcPoints(const CircleArc &arc, float radius,
                      float degrees_per_segment);

  protected:
    PolyLine() {}
    virtual void AddFields() override;
    virtual Bounds GetUntransformedBounds() const override;
    virtual bool IntersectUntransformedRay(const Ray &ray,
                                           Hit &hit) const override;
    virtual ion::gfx::ShapePtr CreateSpecificIonShape() override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::VField<Point3f> points_;
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
