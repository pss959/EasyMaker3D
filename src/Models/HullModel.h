#pragma once

#include "Models/CombinedModel.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(HullModel);

/// HullModel is a derived CombinedModel class that represents the convex hull
/// of any number of operand Models.
///
/// \ingroup Models
class HullModel : public CombinedModel {
  protected:
    HullModel() {}
    virtual TriMesh CombineMeshes(const std::vector<TriMesh> &meshes) override;

  private:
    friend class Parser::Registry;
};
