#pragma once

#include "Models/PrimitiveModel.h"

namespace Parser { class Registry; }

/// SphereModel is a derived PrimitiveModel class representing a sphere with a
/// radius of 1
///
/// The sphere is composed of a number of latitudinal bands divided into
/// sectors. The number of bands and sectors are both dependent on complexity.
///
/// \ingroup Models
class SphereModel : public PrimitiveModel {
  public:
    /// SphereModel responds to complexity.
    virtual bool CanSetComplexity() const override { return true; }

  protected:
    SphereModel() {}
    virtual TriMesh BuildMesh() override;

    friend class Parser::Registry;
};

typedef std::shared_ptr<SphereModel> SphereModelPtr;
