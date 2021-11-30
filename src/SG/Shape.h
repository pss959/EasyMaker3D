#pragma once

#include <ion/gfx/shape.h>

#include "Math/Types.h"
#include "SG/Change.h"
#include "SG/Hit.h"
#include "SG/Object.h"
#include "Util/Assert.h"
#include "Util/Notifier.h"

namespace SG {

/// Abstract base class for all shapes; it wraps an Ion Shape. Derived classes
/// create specific types.
class Shape : public Object {
  public:
    /// Creates, stores, and returns the Ion Shape.
    ion::gfx::ShapePtr SetUpIon();

    /// Returns the Ion Shape for this instance. This will be null until
    /// SetUpIon() is called.
    ion::gfx::ShapePtr GetIonShape() const { return ion_shape_; }

    /// Returns the current Bounds.
    const Bounds & GetBounds();

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
    virtual void ProcessChange(Change change) override;

    /// Returns the current bounds, asserting that they are valid.
    const Bounds & GetValidBounds() const {
        ASSERT(bounds_valid_);
        return bounds_;
    }

  private:
    ion::gfx::ShapePtr ion_shape_;  /// Associated Ion Shape.

    Bounds bounds_;                /// Computed and cached Bounds.
    bool   bounds_valid_ = false;  /// Bounds validity flag.
};

}  // namespace SG
