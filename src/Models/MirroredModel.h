﻿#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"
#include "Models/ConvertedModel.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(MirroredModel);

/// MirroredModel is a derived ConvertedModel class that represents a Model
/// that has been mirrored across one or more planes.
///
/// \ingroup Models
class MirroredModel : public ConvertedModel {
  public:
    /// Adds a Plane (specified in object coordinates) to mirror across.
    void AddPlane(const Plane &plane);

    /// Removes the last Plane added. This should not be called if there is
    /// only one Plane.
    void RemoveLastPlane();

    /// Returns the current mirroring Planes. This may be an empty vector.
    const std::vector<Plane> & GetPlanes() const { return planes_; }

  protected:
    MirroredModel() {}
    virtual void AddFields() override;
    virtual TriMesh ConvertMesh(const TriMesh &original_mesh) override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::VField<Plane> planes_{"planes"};
    ///@}

    friend class Parser::Registry;
};
