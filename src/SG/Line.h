#pragma once

#include "Parser/ObjectSpec.h"
#include "SG/Math.h"
#include "SG/Shape.h"

namespace SG {

//! Line is a derived Shape that represents a 3D line (segment). It does not
//! define normals or texture coordinates, so use a shader that does not
//! require them. It does not respond to intersection queries.
class Line : public Shape {
  public:
    const Point3f & GetEnd0() const { return end0_; }
    const Point3f & GetEnd1() const { return end1_; }

    // Changes the endpoints.
    void SetEndpoints(const Point3f &end0, const Point3f &end1);

    virtual Bounds ComputeBounds() const override;
    virtual bool IntersectRay(const Ray &ray, Hit &hit) const override;
    virtual ion::gfx::ShapePtr CreateIonShape() override;

    static Parser::ObjectSpec GetObjectSpec();

  private:
    //! \name Parsed fields.
    //!@{
    Point3f end0_{0, 0, 0};
    Point3f end1_{1, 0, 0};
    //!@}
};

}  // namespace SG
