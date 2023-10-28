#pragma once

#include <vector>

#include "Math/Polygon.h"
#include "Math/Types.h"
#include "SG/TriMeshShape.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(Polygon);

/// Polygon is a derived PrimitiveShape that represents a 2D polygon in the Z=0
/// plane. It is derived from TriMeshShape to make rendering and intersection
/// testing easier.
///
/// \ingroup SG
class Polygon : public TriMeshShape {
  public:
    /// Defines the Polygon from a Math Polygon.
    void SetPolygon(const ::Polygon &polygon);

    /// Returns a list of all points in the Polygon.
    const std::vector<Point2f> & GetPoints() const { return points_; }

    /// Returns a list of counts of points in each border. The size of this
    /// list indicates the number of borders; it should always be at least 1.
    const std::vector<size_t> & GetBorderCounts() const {
        return border_counts_;
    }

  protected:
    Polygon() {}

    virtual void AddFields() override;
    virtual ion::gfx::ShapePtr CreateSpecificIonShape() override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::VField<Point2f> points_;
    Parser::VField<size_t>  border_counts_;
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
