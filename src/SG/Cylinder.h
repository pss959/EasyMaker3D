#pragma once

#include "SG/TriMeshShape.h"

namespace SG {

//! Cylinder is a derived Shape that represents a cylinder.
class Cylinder : public Shape {
  public:
    virtual void AddFields() override;

    float GetBottomRadius()   const { return bottom_radius_;    }
    float GetTopRadius()      const { return top_radius_;       }
    float GetHeight()         const { return height_;           }
    bool  HasTopCap()         const { return has_top_cap_;      }
    bool  HasBottomCap()      const { return has_bottom_cap_;   }
    int   GetShaftBandCount() const { return shaft_band_count_; }
    int   GetCapBandCount()   const { return cap_band_count_;   }
    int   GetSectorCount()    const { return sector_count_;     }

    virtual bool IntersectRay(const Ray &ray, Hit &hit) const override;

  protected:
    virtual Bounds ComputeBounds() const override;
    virtual ion::gfx::ShapePtr CreateIonShape() override;

  private:
    //! \name Parsed Fields
    //!@{
    Parser::TField<float> bottom_radius_{"bottom_radius"};
    Parser::TField<float> top_radius_{"top_radius"};
    Parser::TField<float> height_{"height"};
    Parser::TField<bool>  has_top_cap_{"has_top_cap"};
    Parser::TField<bool>  has_bottom_cap_{"has_bottom_cap"};
    Parser::TField<int>   shaft_band_count_{"shaft_band_count"};
    Parser::TField<int>   cap_band_count_{"cap_band_count"};
    Parser::TField<int>   sector_count_{"sector_count"};
    //!@}
};

}  // namespace SG
