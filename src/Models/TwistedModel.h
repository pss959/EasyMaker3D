#pragma once

#include "Base/Memory.h"
#include "Math/SlicedMesh.h"
#include "Math/Types.h"
#include "Models/SpinBasedModel.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(TwistedModel);

/// TwistedModel is a derived ConvertedModel class that represents a Model that
/// has been twisted by a Spin specified in object coordinates of the operand
/// Model.
///
/// \ingroup Models
class TwistedModel : public SpinBasedModel {
  protected:
    TwistedModel() {}

    virtual TriMesh ConvertMesh(const TriMesh &mesh) override;

  private:
    /// Caches the operand Model mesh split into slices based on complexity and
    /// the current Spin axis.
    SlicedMesh sliced_mesh_;

    /// Complexity used to create #sliced_mesh_.
    float      sliced_complexity_ = -1;

    /// Axis used to create #sliced_mesh_.
    Vector3f   sliced_axis_{0, 0, 0};

    friend class Parser::Registry;
};
