#include "Math/PolyMeshBuilder.h"

#include "Util/Assert.h"

size_t PolyMeshBuilder::AddVertex(const Point3f &point) {
    vertices_.push_back(point);
    return vertices_.size() - 1;
}

const Point3f & PolyMeshBuilder::GetVertex(size_t index) const {
    ASSERT(index < vertices_.size());
    return vertices_[index];
}

void PolyMeshBuilder::MoveVertex(size_t index, const Point3f &new_point) {
    ASSERT(index < vertices_.size());
    vertices_[index] = new_point;
}

void PolyMeshBuilder::AddPolygon(const std::vector<size_t> &indices) {
    ASSERT(indices.size() >= 3);
    PolyMesh::Border border;
    border.indices = indices;
    border.is_hole = false;
    borders_.push_back(border);
}

void PolyMeshBuilder::AddHole(const std::vector<size_t> &indices) {
    ASSERT(indices.size() >= 3);
    ASSERT(! borders_.empty());  // Must have outer border to add hole to.
    PolyMesh::Border border;
    border.indices = indices;
    border.is_hole = true;
    borders_.push_back(border);
}

PolyMesh PolyMeshBuilder::BuildPolyMesh() const {
    return PolyMesh(vertices_, borders_);
}
