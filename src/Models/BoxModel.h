#include "Models/PrimitiveModel.h"

//! BoxModel is a derived PrimitiveModel class representing a box ranging from
//! -1 to 1 in all 3 dimensions. It is not affected by complexity; it always has
//! 6 faces with 2 triangles each.
//!
//! \ingroup Models
class BoxModel : public PrimitiveModel {
  protected:
    virtual TriMesh BuildMesh() override;
};

typedef std::shared_ptr<BoxModel> BoxModelPtr;
