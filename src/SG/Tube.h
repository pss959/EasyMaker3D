#pragma once

#include <vector>

#include "Base/Memory.h"
#include "Math/Curves.h"
#include "Math/Types.h"
#include "SG/TriMeshShape.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(Tube);

/// Tube is a derived PrimitiveShape that represents a set of connected 3D
/// tubular segments. It does not have end caps. This is used primarily to
/// simulate wide lines.
///
/// \ingroup SG
class Tube : public TriMeshShape {
  public:
    const std::vector<Point3f> & GetPoints() const { return points_; }

    float GetDiameter()  const { return diameter_;   }
    int   GetSideCount() const { return side_count_; }

    /// Sets the points forming the backbone of the Tube.
    void SetPoints(const std::vector<Point3f> &points);

    // Sets points to form a single segment between the two given points.
    void SetEndpoints(const Point3f &p0, const Point3f &p1);

    /// Sets points to form an arc. If the arc angle is not large enough, this
    /// clears the points.
    void SetArcPoints(const CircleArc &arc, float radius,
                      float degrees_per_segment);

  protected:
    Tube() {}
    virtual void AddFields() override;
    virtual ion::gfx::ShapePtr CreateSpecificIonShape() override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::VField<Point3f> points_;
    Parser::TField<float>   diameter_;
    Parser::TField<int>     side_count_;
    ///@}

    /// Updates the Ion Shape (if it exists) when fields change.
    void UpdateIonShape_();

    /// Builds a TriMesh using the current field values.
    TriMesh BuildMesh_() const;

    friend class Parser::Registry;
};

}  // namespace SG
