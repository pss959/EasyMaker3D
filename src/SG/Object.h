#pragma once

#include "NParser/Object.h"

namespace SG {

//! Object is an abstract base class for all SG classes. It supports naming.
class Object : public NParser::Object {
  public:
    virtual ~Object() {}

  protected:
    //! The constructor is protected to make this abstract.
    Object() {}
};

}  // namespace SG
