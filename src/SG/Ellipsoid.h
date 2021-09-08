#pragma once

#include "Math/Types.h"
#include "SG/Shape.h"

namespace SG {

//! Ellipsoid is a derived Shape that represents an ellipsoid. It is a sphere
//! with radius .5 by default.
class Ellipsoid : public Shape {
  public:
    virtual void AddFields() override;

    const Anglef &   GetLongitudeStart() const { return longitude_start_; }
    const Anglef &   GetLongitudeEnd()   const { return longitude_end_;   }
    const Anglef &   GetLatitudeStart()  const { return latitude_start_;  }
    const Anglef &   GetLatitudeEnd()    const { return latitude_end_;    }
    int              GetBandCount()      const { return band_count_;      }
    int              GetSectorCount()    const { return sector_count_;    }
    const Vector3f & GetSize()           const { return size_;            }

    virtual bool IntersectRay(const Ray &ray, Hit &hit) const override;

  protected:
    virtual Bounds ComputeBounds() const override;
    virtual ion::gfx::ShapePtr CreateIonShape() override;

  private:
    //! \name Parsed Fields
    //!@{
    Parser::TField<Anglef>   longitude_start_{"longitude_start",
                                              Anglef::FromDegrees(0)};
    Parser::TField<Anglef>   longitude_end_{"longitude_end",
                                            Anglef::FromDegrees(360)};
    Parser::TField<Anglef>   latitude_start_{"latitude_start",
                                             Anglef::FromDegrees(-90)};
    Parser::TField<Anglef>   latitude_end_{"latitude_end",
                                           Anglef::FromDegrees(90)};
    Parser::TField<int>      band_count_{"band_count", 10};
    Parser::TField<int>      sector_count_{"sector_count", 10};
    Parser::TField<Vector3f> size_{"size", {1, 1, 1}};
    //!@}
};

}  // namespace SG
