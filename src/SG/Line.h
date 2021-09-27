#pragma once

#include "Math/Types.h"
#include "SG/Shape.h"

namespace Parser { class Registry; }

namespace SG {

/// Line is a derived Shape that represents a 3D line (segment). It does not
/// define normals or texture coordinates, so use a shader that does not
/// require them. It does not respond to intersection queries.
class Line : public Shape {
  public:
    virtual void AddFields() override;

    const Point3f & GetEnd0() const { return end0_; }
    const Point3f & GetEnd1() const { return end1_; }

    // Changes the endpoints.
    void SetEndpoints(const Point3f &end0, const Point3f &end1);

    virtual bool IntersectRay(const Ray &ray, Hit &hit) const override;

  protected:
    Line() {}
    virtual Bounds ComputeBounds() const override;
    virtual ion::gfx::ShapePtr CreateSpecificIonShape() override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::TField<Point3f> end0_{"end0", {0, 0, 0}};
    Parser::TField<Point3f> end1_{"end1", {1, 0, 0}};
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
