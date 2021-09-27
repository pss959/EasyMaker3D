#pragma once

#include "Math/Types.h"
#include "SG/Shape.h"

namespace Parser { class Registry; }

namespace SG {

/// Box is a derived Shape that represents a box. It is 1x1x1 by default.
class Box : public Shape {
  public:
    virtual void AddFields() override;

    /// Returns the 3D size of the box.
    const Vector3f & GetSize() const { return size_; }

    virtual bool IntersectRay(const Ray &ray, Hit &hit) const override;

  protected:
    Box() {}
    virtual Bounds ComputeBounds() const override;
    virtual ion::gfx::ShapePtr CreateSpecificIonShape() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Vector3f> size_{"size", {1, 1, 1}};
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
