#pragma once

#include <ion/gfx/shape.h>

#include "Parser/ObjectSpec.h"
#include "SG/Change.h"
#include "SG/Hit.h"
#include "SG/Math.h"
#include "SG/Object.h"
#include "Util/Notification.h"

namespace SG {

//! Abstract base class for all shapes; it wraps an Ion Shape. Derived classes
//! create specific types.
class Shape : public Object, public Util::Notifier<Change> {
  public:
    //! Returns the associated Ion shape.
    const ion::gfx::ShapePtr & GetIonShape() const { return ion_shape_; }

    //! Returns the current Bounds.
    const Bounds & GetBounds();

    //! Intersects the Shape with the given ray (in local coordinates). If
    //! there is any intersection, this sets the distance, point, and normal
    //! fields in the given Hit.
    virtual bool IntersectRay(const Ray &ray, Hit &hit) const = 0;

    virtual void SetUpIon(IonContext &context) override;

    static Parser::ObjectSpec GetObjectSpec();

  protected:
    //! The constructor is protected to make this abstract.
    Shape() {}

    //! Derived classes must implement this to compute and return bounds in
    //! local coordinates.
    virtual Bounds ComputeBounds() const = 0;

    //! Derived classes must implement this to create the Ion Shape when
    //! necessary.
    virtual ion::gfx::ShapePtr CreateIonShape() = 0;

  private:
    ion::gfx::ShapePtr ion_shape_;  //! Associated Ion Shape.

    Bounds bounds_;                //! Computed and cached Bounds.
    bool   bounds_valid_ = false;  //! Bounds validity flag.
};

}  // namespace SG
