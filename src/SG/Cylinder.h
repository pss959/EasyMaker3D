#pragma once

#include "SG/Shape.h"

namespace Parser { class Registry; }

namespace SG {

/// Cylinder is a derived Shape that represents a cylinder aligned with the Y
/// axis. It is 1x1x1 by default.
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
    Cylinder() {}
    virtual Bounds ComputeBounds() const override;
    virtual ion::gfx::ShapePtr CreateSpecificIonShape() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<float> bottom_radius_{"bottom_radius", .5f};
    Parser::TField<float> top_radius_{"top_radius", .5f};
    Parser::TField<float> height_{"height", 1.f};
    Parser::TField<bool>  has_top_cap_{"has_top_cap", true};
    Parser::TField<bool>  has_bottom_cap_{"has_bottom_cap", true};
    Parser::TField<int>   shaft_band_count_{"shaft_band_count", 1};
    Parser::TField<int>   cap_band_count_{"cap_band_count", 1};
    Parser::TField<int>   sector_count_{"sector_count", 10};
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
