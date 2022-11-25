#pragma once

#include <vector>

#include "Math/Types.h"

/// A BeveledMesh represents a mesh that contains special information to help
/// apply different bevel profiles. Typical use of a BeveledMesh are:
///
///  - Create a BeveledMesh from a TriMesh and a maximum angle used to select
///    which edges to bevel. This uses a default profile and scale.
///  - Use GetResultMesh() to retrieve the current result TriMesh.
///  - Changing the profile and/or scale modifies the BeveledMesh without
///    having to recreate the whole thing.
///  - Changing the maximum angle requires recreating the BeveledMesh.
///
/// \ingroup Math
class BeveledMesh {
  public:
    /// Constructs a BeveledMesh from a TriMesh and a maximum angle used to
    /// select which edges to bevel. This uses a default profile and scale.
    BeveledMesh(const TriMesh &mesh, const Anglef &max_angle);

    /// Returns the original TriMesh.
    const TriMesh & GetOriginalMesh() const { return original_mesh_; }

    /// Returns a TriMesh representing the beveled mesh.
    const TriMesh & GetResultMesh()   const { return result_mesh_; }

  private:
    class Helper_;

    TriMesh original_mesh_;
    Anglef  max_angle_;
    TriMesh result_mesh_;
};
