#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"
#include "Models/ConvertedModel.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ClippedModel);

/// ClippedModel is a derived ConvertedModel class that represents a Model
/// that has been clipped by one or more planes.
///
/// \ingroup Models
class ClippedModel : public ConvertedModel {
  public:
    /// Adds a Plane (specified in object coordinates) to clip to. The side the
    /// Plane normal points toward is clipped away.
    void AddPlane(const Plane &plane);

    /// Removes the last Plane added. This should not be called if there is
    /// only one Plane.
    void RemoveLastPlane();

    /// Returns the current clipping Planes. This may be an empty vector.
    const std::vector<Plane> & GetPlanes() const { return planes_; }

  protected:
    ClippedModel() {}
    virtual void AddFields() override;
    virtual TriMesh ConvertMesh(const TriMesh &original_mesh) override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::VField<Plane> planes_{"planes"};
    ///@}

    friend class Parser::Registry;
};
