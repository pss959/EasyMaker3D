#pragma once

struct Bevel;
struct TriMesh;

/// The Beveler class creates beveled edges for a TriMesh. The bevel data is
/// supplied in a Bevel instance.
///
/// \ingroup Math
class Beveler {
  public:
    /// Applies the given Bevel to a TriMesh, returning the beveled TriMesh.
    static TriMesh ApplyBevel(const TriMesh &mesh, const Bevel &bevel);
};
