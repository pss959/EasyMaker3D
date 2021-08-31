#pragma once

#include "Parser/ObjectSpec.h"
#include "SG/Math.h"
#include "SG/Shape.h"

namespace SG {

//! Cylinder is a derived Shape that represents a cylinder.
class Cylinder : public Shape {
  public:
    float GetBottomRadius()   const { return bottom_radius_;    }
    float GetTopRadius()      const { return top_radius_;       }
    float GetHeight()         const { return height_;           }
    bool  HasTopCap()         const { return has_top_cap_;      }
    bool  HasBottomCap()      const { return has_bottom_cap_;   }
    int   GetShaftBandCount() const { return shaft_band_count_; }
    int   GetCapBandCount()   const { return cap_band_count_;   }
    int   GetSectorCount()    const { return sector_count_;     }

    virtual Bounds ComputeBounds() const override;
    virtual bool IntersectRay(const Ray &ray, Hit &hit) const override;
    virtual ion::gfx::ShapePtr CreateIonShape() override;

    static Parser::ObjectSpec GetObjectSpec();

  private:
    //! Triangle mesh for the Ion shape.
    TriMesh tri_mesh_;

    //! \name Parsed Fields
    //!@{
    float bottom_radius_    = 1.f;
    float top_radius_       = 1.f;
    float height_           = 2.f;
    bool  has_top_cap_      = true;
    bool  has_bottom_cap_   = true;
    int   shaft_band_count_ = 1;
    int   cap_band_count_   = 1;
    int   sector_count_     = 10;
    //!@}

    //! Fills the tri_mesh_ struct with triangles from the Ion shape.
    void FillTriMesh_(const ion::gfx::Shape &shape);
};

}  // namespace SG
