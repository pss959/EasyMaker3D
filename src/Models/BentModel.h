//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Enums/Dim.h"
#include "Math/SlicedMesh.h"
#include "Math/Types.h"
#include "Models/SpinBasedModel.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(BentModel);

/// BentModel is a derived ConvertedModel class that represents a Model that
/// has been bent by some angle around an arbitrary axis through an arbitrary
/// center point with an optional offset proportional to the angle.
///
/// \ingroup Models
class BentModel : public SpinBasedModel {
  protected:
    BentModel() {}

    virtual TriMesh ConvertMesh(const TriMesh &mesh) override;

  private:
    /// Caches the operand Model mesh split into slices based on complexity and
    /// the current Spin axis.
    SlicedMesh sliced_mesh_;

    /// Complexity used to create #sliced_mesh_.
    float      sliced_complexity_ = -1;

    /// Axis used to create #sliced_mesh_.
    Vector3f   sliced_axis_{0, 0, 0};

    /// Returns the principal dimension to slice along for the given Spin axis
    /// applied to the given TriMesh.
    static Dim GetSliceDim_(const TriMesh &mesh, const Vector3f &spin_axis);

    friend class Parser::Registry;
};
