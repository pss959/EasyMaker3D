#pragma once

#include <ion/gfx/shape.h>

#include "Math/Types.h"
#include "SG/Change.h"
#include "SG/Object.h"
#include "Util/Assert.h"
#include "Util/Memory.h"

namespace SG {

DECL_SHARED_PTR(IonContext);
DECL_SHARED_PTR(Shape);
struct Hit;

/// Abstract base class for all shapes; it wraps an Ion Shape. Derived classes
/// create specific types.
///
/// \ingroup SG
class Shape : public Object {
  public:
    /// Creates, stores, and returns the Ion Shape.
    ion::gfx::ShapePtr SetUpIon(const IonContextPtr &ion_context);

    /// Returns the Ion Shape for this instance. This will be null until
    /// SetUpIon() is called.
    ion::gfx::ShapePtr GetIonShape() const { return ion_shape_; }

    /// Returns the current Bounds.
    const Bounds & GetBounds() const;

    /// Intersects the Shape with the given ray (in local coordinates). If
    /// there is any intersection, this sets the distance, point, and normal
    /// fields in the given Hit.
    virtual bool IntersectRay(const Ray &ray, Hit &hit) const = 0;

  protected:
    /// The constructor is protected to make this abstract.
    Shape() {}

    /// Derived classes must implement this to compute and return bounds in
    /// local coordinates.
    virtual Bounds ComputeBounds() const = 0;

    /// Derived classes must implement this to create and return the Ion Shape
    /// when necessary.
    virtual ion::gfx::ShapePtr CreateSpecificIonShape() = 0;

    /// Redefines this to mark bounds as invalid.
    virtual bool ProcessChange(Change change, const Object &obj) override;

    /// Returns the current bounds, asserting that they are valid.
    const Bounds & GetValidBounds() const {
        ASSERT(bounds_valid_);
        return bounds_;
    }

  private:
    ion::gfx::ShapePtr ion_shape_;  ///< Associated Ion Shape.

    // These are mutable because they are caches.
    mutable Bounds bounds_;                ///< Computed and cached Bounds.
    mutable bool   bounds_valid_ = false;  ///< Bounds validity flag.
};

}  // namespace SG
