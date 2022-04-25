#pragma once

#include <vector>

#include "Math/Polygon.h"
#include "Math/Types.h"
#include "Memory.h"
#include "SG/TriMeshShape.h"

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

  protected:
    Polygon() {}

    virtual void AddFields() override;
    virtual ion::gfx::ShapePtr CreateSpecificIonShape() override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::VField<Point2f> points_{"points"};
    Parser::VField<size_t>  border_counts_{"border_counts"};
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
