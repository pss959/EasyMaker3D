#pragma once

#include <ion/gfxutils/shapeutils.h>

#include "Math/Types.h"
#include "SG/TriMeshShape.h"

namespace Parser { class Registry; }

namespace SG {

/// RegularPolygon is a derived TriMeshShape that represents a regular
/// polygon. It is an equilateral triangle in the Z=0 plane by default. This is
/// derived from TriMeshShape to make intersection testing easier.
class RegularPolygon : public TriMeshShape {
  public:
    typedef ion::gfxutils::PlanarShapeSpec::PlaneNormal PlaneNormal;

    int         GetSides()       const { return sides_;        }
    PlaneNormal GetPlaneNormal() const { return plane_normal_; }

  protected:
    RegularPolygon() {}
    virtual void AddFields() override;
    virtual Bounds ComputeBounds() const override;
    virtual ion::gfx::ShapePtr CreateSpecificIonShape() override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::TField<int>            sides_{"sides", 3};
    Parser::EnumField<PlaneNormal> plane_normal_{"plane_normal",
                                                 PlaneNormal::kPositiveZ};
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
