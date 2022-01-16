#pragma once

#include <vector>

#include "Math/Types.h"
#include "SG/PrimitiveShape.h"

namespace Parser { class Registry; }

namespace SG {

/// PolyLine is a derived PrimitiveShape that represents a set of connected 3D
/// line segments. It does not define normals or texture coordinates, so use a
/// shader that does not require them. It does not respond to intersection
/// queries.
class PolyLine : public PrimitiveShape {
  public:
    const std::vector<Point3f> & GetPoints() const { return points_; }

    void SetPoints(const std::vector<Point3f> &points);

    virtual bool IntersectRay(const Ray &ray, Hit &hit) const override;

  protected:
    PolyLine() {}
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual Bounds GetUntransformedBounds() const override;
    virtual ion::gfx::ShapePtr CreateSpecificIonShape() override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::VField<Point3f> points_{"points"};
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
