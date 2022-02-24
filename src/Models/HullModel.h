#pragma once

#include "Models/CombinedModel.h"

namespace Parser { class Registry; }

/// HullModel is a derived CombinedModel class that represents the convex hull
/// of any number of operand Models.
///
/// \ingroup Models
class HullModel : public CombinedModel {
  protected:
    HullModel() {}
    virtual TriMesh BuildMesh() override;

  private:
    friend class Parser::Registry;
};

typedef std::shared_ptr<HullModel> HullModelPtr;
