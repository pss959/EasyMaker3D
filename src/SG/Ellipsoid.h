#pragma once

#include "Parser/ObjectSpec.h"
#include "SG/Math.h"
#include "SG/Shape.h"

namespace SG {

//! Ellipsoid is a derived Shape that represents an ellipsoid.
class Ellipsoid : public Shape {
  public:
    const Anglef &   GetLongitudeStart() const { return longitude_start_; }
    const Anglef &   GetLongitudeEnd()   const { return longitude_end_;   }
    const Anglef &   GetLatitudeStart()  const { return latitude_start_;  }
    const Anglef &   GetLatitudeEnd()    const { return latitude_end_;    }
    int              GetBandCount()      const { return band_count_;      }
    int              GetSectorCount()    const { return sector_count_;    }
    const Vector3f & GetSize()           const { return size_;            }

    virtual Bounds             ComputeBounds()  override;
    virtual ion::gfx::ShapePtr CreateIonShape() override;

    static Parser::ObjectSpec GetObjectSpec();

  private:
    //! \name Parsed Fields
    //!@{
    Anglef   longitude_start_ = Anglef::FromDegrees(0.f);
    Anglef   longitude_end_   = Anglef::FromDegrees(360.f);
    Anglef   latitude_start_  = Anglef::FromDegrees(-90.f);
    Anglef   latitude_end_    = Anglef::FromDegrees(90.f);
    int      band_count_      = 10;
    int      sector_count_    = 10;
    Vector3f size_ { 2.f, 2.f, 2.f };
    //!@}
};

}  // namespace SG
