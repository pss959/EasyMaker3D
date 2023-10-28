#pragma once

#include "SG/PrimitiveShape.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(Cylinder);

/// Cylinder is a derived PrimitiveShape that represents a cylinder aligned
/// with the Y axis. It is 1x1x1 by default.
///
/// \ingroup SG
class Cylinder : public PrimitiveShape {
  public:
    float GetBottomRadius()   const { return bottom_radius_;    }
    float GetTopRadius()      const { return top_radius_;       }
    float GetHeight()         const { return height_;           }
    bool  HasTopCap()         const { return has_top_cap_;      }
    bool  HasBottomCap()      const { return has_bottom_cap_;   }
    int   GetShaftBandCount() const { return shaft_band_count_; }
    int   GetCapBandCount()   const { return cap_band_count_;   }
    int   GetSectorCount()    const { return sector_count_;     }

  protected:
    Cylinder() {}
    virtual void AddFields() override;
    virtual Bounds GetUntransformedBounds() const override;
    virtual bool IntersectUntransformedRay(const Ray &ray,
                                           Hit &hit) const override;
    virtual ion::gfx::ShapePtr CreateSpecificIonShape() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<float> bottom_radius_;
    Parser::TField<float> top_radius_;
    Parser::TField<float> height_;
    Parser::TField<bool>  has_top_cap_;
    Parser::TField<bool>  has_bottom_cap_;
    Parser::TField<int>   shaft_band_count_;
    Parser::TField<int>   cap_band_count_;
    Parser::TField<int>   sector_count_;
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
