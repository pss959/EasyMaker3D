#pragma once

#include "SG/TriMeshShape.h"

namespace Parser { class Registry; }

namespace SG {

/// Torus is a derived TriMeshShape that represents a torus aligned with the Y
/// axis, defined by inner and outer radii.
class Torus : public TriMeshShape {
  public:
    float GetOuterRadius() const { return outer_radius_; }
    float GetInnerRadius() const { return inner_radius_; }
    int   GetRingCount()   const { return ring_count_;   }
    int   GetSectorCount() const { return sector_count_; }

    /// Sets the inner radius of the torus.
    void SetInnerRadius(float radius) { inner_radius_ = radius; }

    /// Sets the outer radius of the torus.
    void SetOuterRadius(float radius) { outer_radius_ = radius; }

  protected:
    Torus() {}
    virtual void AddFields() override;
    virtual Bounds ComputeBounds() const override;
    virtual ion::gfx::ShapePtr CreateSpecificIonShape() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<float> outer_radius_{"outer_radius", 0.9f};
    Parser::TField<float> inner_radius_{"inner_radius", 0.1f};
    Parser::TField<int>   ring_count_{"ring_count", 20};
    Parser::TField<int>   sector_count_{"sector_count", 20};
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
