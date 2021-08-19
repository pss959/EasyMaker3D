#pragma once

#include "NParser/ObjectSpec.h"
#include "SG/Math.h"
#include "SG/Shape.h"

namespace SG {

//! Box is a derived Shape that represents a box.
class Box : public Shape {
  public:
    // XXXX
    const Vector3f & GetSize() const { return size_; }

    static NParser::ObjectSpec GetObjectSpec();

  private:
    // Parsed fields.
    Vector3f size_{ 2.f, 2.f, 2.f };
};

}  // namespace SG
