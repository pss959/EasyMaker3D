#pragma once

#include "Math/Bevel.h"
#include "Math/PolyMesh.h"
#include "Math/Types.h"

//! The Beveler class creates beveled edges for meshes. The bevel data is
//! supplied in a Bevel instance.
//!
//! \ingroup Math
class Beveler {
  public:
    //! Applies the given Bevel to a TriMesh, returning the beveled TriMesh.
    static TriMesh ApplyBevel(const TriMesh &mesh, const Bevel &bevel);

    //! Applies the given Bevel to a PolyMesh, returning the beveled PolyMesh.
    //! This assumes the PolyMesh has had coplanar faces merged.
    static PolyMesh ApplyBevel(const PolyMesh &poly_mesh, const Bevel &bevel);
};
