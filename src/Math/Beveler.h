#pragma once

#include "Math/Bevel.h"

//! The Beveler class creates beveled edges for meshes. The bevel data is
//! supplied in a Bevel instance.
//!
//! \ingroup Math
class Beveler {
  public:
    //! Applies the given Bevel to a TriMesh, returning the beveled TriMesh.
    static TriMesh ApplyBevel(const TriMesh &mesh, const Bevel &bevel);
};
