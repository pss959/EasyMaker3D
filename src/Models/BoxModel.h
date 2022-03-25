#pragma once

#include "Memory.h"
#include "Models/PrimitiveModel.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(BoxModel);

/// BoxModel is a derived PrimitiveModel class representing a box ranging from
/// -1 to 1 in all 3 dimensions. It is not affected by complexity; it always has
/// 6 faces with 2 triangles each.
///
/// \ingroup Models
class BoxModel : public PrimitiveModel {
  protected:
    BoxModel() {}
    virtual TriMesh BuildMesh() override;

    friend class Parser::Registry;
};
