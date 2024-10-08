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

DECL_SHARED_PTR(Box);

/// Box is a derived PrimitiveShape that represents a box. It is 1x1x1 by
/// default.
///
/// \ingroup SG
class Box : public PrimitiveShape {
  public:
    /// Returns the 3D size of the box.
    const Vector3f & GetSize() const { return size_; }

  protected:
    Box() {}
    virtual void AddFields() override;
    virtual Bounds GetUntransformedBounds() const override;
    virtual bool IntersectUntransformedRay(const Ray &ray,
                                           Hit &hit) const override;
    virtual ion::gfx::ShapePtr CreateSpecificIonShape() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Vector3f> size_;
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
