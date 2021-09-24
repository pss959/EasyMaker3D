#pragma once

#include <ion/gfx/shape.h>

#include "Assert.h"
#include "Math/Types.h"
#include "SG/Change.h"
#include "SG/Hit.h"
#include "SG/Object.h"
#include "Util/Notifier.h"

namespace SG {

/// Abstract base class for all shapes; it wraps an Ion Shape. Derived classes
/// create specific types.
class Shape : public Object {
  public:
    /// Returns the Ion Shape for this instance. This will be null until
    /// CreateIonShape() is called.
    ion::gfx::ShapePtr GetIonShape() const { return ion_shape_; }

    /// Creates and stores an Ion Shape.
    void CreateIonShape();

    /// Returns a Notifier that is invoked when a change is made to the shape.
    Util::Notifier<Change> & GetChanged() { return changed_; }

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

    /// Returns the current bounds, asserting that they are valid.
    const Bounds & GetValidBounds() const {
        ASSERT(bounds_valid_);
        return bounds_;
    }

  private:
    ion::gfx::ShapePtr ion_shape_;  /// Associated Ion Shape.

    Bounds bounds_;                /// Computed and cached Bounds.
    bool   bounds_valid_ = false;  /// Bounds validity flag.

    /// Notifies when a change is made to the shape.
    Util::Notifier<Change> changed_;
};

}  // namespace SG
