#pragma once

#include "NParser/FieldSpec.h"
#include "SG/Math.h"
#include "SG/Shape.h"

namespace SG {

//! Ellipsoid is a derived Shape that represents an ellipsoid.
class Ellipsoid : public Shape {
  public:
    static std::vector<NParser::FieldSpec> GetFieldSpecs();

  private:
    // Parsed fields.
    Anglef   longitude_start_ = Anglef::FromDegrees(0.f);
    Anglef   longitude_end_   = Anglef::FromDegrees(360.f);
    Anglef   latitude_start_  = Anglef::FromDegrees(-90.f);
    Anglef   latitude_end_    = Anglef::FromDegrees(90.f);
    int      band_count_      = 10;
    int      sector_count_    = 10;
    Vector3f size_ { 2.f, 2.f, 2.f };
};

}  // namespace SG
