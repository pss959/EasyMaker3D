#include "Models/PrimitiveModel.h"

//! TorusModel is a derived PrimitiveModel class representing a torus. It
//! allows the outer and inner radii to be set. It is defined to be centered
//! around the origin with symmetry along the Y axis. Setting the inner radius
//! will force the outer radius to be larger if necessary to prevent
//! self-intersections. By default, the outer radius is 1 and the inner radius
//! is 0.2.
//!
//! A complexity 0 torus has 3 sectors with 3 vertices around each ring, while
//! a complexity 1 torus has 100 sectors and rings.
//!
//! \ingroup Models
class TorusModel : public PrimitiveModel {
  public:
    virtual void AddFields() override;

    //! Sets the inner radius, clamped to be at least kMinInnerRadius. Also
    //! increases outer radius if necessary to make sure the hole radius is at
    //! least kMinHoleRadius.
    void SetInnerRadius(float radius);

    //! Sets the outer radius, clamped to be at least the value returned by
    //! GetMinOuterRadius(). To make it smaller, set the inner radius first.
    void SetOuterRadius(float radius);

    //! Returns the current inner radius.
    float GetInnerRadius() const { return inner_radius_; }

    //! Returns the current outer radius.
    float GetOuterRadius() const { return outer_radius_; }

    //! Returns the smallest value the outer radius can have based on the
    //! current inner radius.
    float GetMinOuterRadius() const {
        return GetMinOuterRadiusForInnerRadius(inner_radius_);
    }

    //! Returns the smallest value for the outer radius  based on the
    //! given inner radius.
    static float GetMinOuterRadiusForInnerRadius(float inner_radius) {
        return 2 * inner_radius + kMinHoleRadius;
    }

    //! TorusModel responds to complexity.
    virtual bool CanSetComplexity() const override { return true; }

    //! Minimum inner radius value.
    static constexpr float kMinInnerRadius = .01f;

    //! Minimum radius of the hole.
    static constexpr float kMinHoleRadius  = .01f;

  protected:
    virtual TriMesh BuildMesh() override;

  private:
    //! \name Parsed fields.
    //!@{
    Parser::TField<float> inner_radius_{"inner_radius", .2f};
    Parser::TField<float> outer_radius_{"outer_radius", 1};
    //!@}
};
