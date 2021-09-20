#pragma once

#include "Models/ParentModel.h"

//! RootModel is a derived ParentModel that is used only as the root containing
//! all Models in the scene. It does not supply a mesh because it is not
//! rendered directly.
//!
//! \ingroup Models
class RootModel : public ParentModel {
  public:
    virtual TriMesh BuildMesh() { return TriMesh(); }
};

typedef std::shared_ptr<RootModel> RootModelPtr;
