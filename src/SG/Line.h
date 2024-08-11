//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Math/Types.h"
#include "SG/PrimitiveShape.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(Line);

/// Line is a derived PrimitiveShape that represents a 3D line (segment). It
/// does not define normals or texture coordinates, so use a shader that does
/// not require them. It does not respond to intersection queries.
///
/// \ingroup SG
class Line : public PrimitiveShape {
  public:
    const Point3f & GetEnd0() const { return end0_; }
    const Point3f & GetEnd1() const { return end1_; }

    // Changes the endpoints.
    void SetEndpoints(const Point3f &end0, const Point3f &end1);

  protected:
    Line() {}
    virtual void AddFields() override;
    virtual Bounds GetUntransformedBounds() const override;
    virtual bool IntersectUntransformedRay(const Ray &ray,
                                           Hit &hit) const override;
    virtual ion::gfx::ShapePtr CreateSpecificIonShape() override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::TField<Point3f> end0_;
    Parser::TField<Point3f> end1_;
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
