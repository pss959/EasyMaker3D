#pragma once

#include <vector>

#include "Math/PolyMesh.h"
#include "Math/Types.h"

/// A PolyMeshBuilder can be used to construct a PolyMesh instance
/// incrementally.
//
// \ingroup Math
class PolyMeshBuilder {
  public:
    /// Adds a vertex at the given point, returning its index.
    size_t AddVertex(const Point3f &point);

    /// Changes the position of a vertex.
    void MoveVertex(size_t index, const Point3f &new_point);

    /// Returns the current position of the indexed vertex.
    const Point3f & GetVertex(size_t index) const;

    /// Adds a polygon constructed from 3 or more outer border indices.
    void AddPolygon(const std::vector<GIndex> &indices);

    /// Adds a hole in the last polygon added.
    void AddHole(const std::vector<GIndex> &indices);

    /// Convenience to add a triangle constructed from 3 indices, assumed to be
    /// an outer border.
    void AddTriangle(GIndex i0, GIndex i1, GIndex i2) {
        AddPolygon(std::vector<GIndex>({i0, i1, i2}));
    }

    /// Convenience to add a quad constructed from 4 indices, assumed to be an
    /// outer border.
    void AddQuad(GIndex i0, GIndex i1, GIndex i2, GIndex i3) {
        AddPolygon(std::vector<GIndex>({i0, i1, i2, i3}));
    }

    /// Constructs and returns a PolyMesh from the data.
    PolyMesh BuildPolyMesh() const;

  private:
    std::vector<Point3f>          vertices_;  ///< Vertex storage.
    std::vector<PolyMesh::Border> borders_;   ///< Border storage.
};

