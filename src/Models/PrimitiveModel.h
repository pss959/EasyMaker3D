#pragma once

#include "Models/Model.h"

//! PrimitiveModel is an abstract derived Model class representing a primitive
//! Model of some sort. There are derived classes for each specific primitive
//! type.
//!
//! \ingroup Models
class PrimitiveModel : public Model {
  protected:
    //! Protected constructor to make this abstract.
    PrimitiveModel() {}
};
