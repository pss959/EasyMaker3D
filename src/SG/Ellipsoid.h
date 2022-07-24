#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"
#include "SG/PrimitiveShape.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(Ellipsoid);

/// Ellipsoid is a derived PrimitiveShape that represents an ellipsoid. It is a
/// sphere with radius .5 by default.
///
/// \ingroup SG
class Ellipsoid : public PrimitiveShape {
  public:
    const Anglef &   GetLongitudeStart() const { return longitude_start_; }
    const Anglef &   GetLongitudeEnd()   const { return longitude_end_;   }
    const Anglef &   GetLatitudeStart()  const { return latitude_start_;  }
    const Anglef &   GetLatitudeEnd()    const { return latitude_end_;    }
    int              GetBandCount()      const { return band_count_;      }
    int              GetSectorCount()    const { return sector_count_;    }
    const Vector3f & GetSize()           const { return size_;            }

  protected:
    Ellipsoid() {}
    virtual void AddFields() override;
    virtual Bounds GetUntransformedBounds() const override;
    virtual bool IntersectUntransformedRay(const Ray &ray,
                                           Hit &hit) const override;
    virtual ion::gfx::ShapePtr CreateSpecificIonShape() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Anglef>   longitude_start_;
    Parser::TField<Anglef>   longitude_end_;
    Parser::TField<Anglef>   latitude_start_;
    Parser::TField<Anglef>   latitude_end_;
    Parser::TField<int>      band_count_;
    Parser::TField<int>      sector_count_;
    Parser::TField<Vector3f> size_;
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
