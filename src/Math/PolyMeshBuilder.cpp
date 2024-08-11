//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Math/PolyMeshBuilder.h"

#include "Util/Assert.h"
#include "Util/KLog.h"
#include "Util/String.h"

GIndex PolyMeshBuilder::AddVertex(const Point3f &point) {
    KLOG('l', "PolyMeshBuilder added vertex " << vertices_.size()
         << " at " << point);
    vertices_.push_back(point);
    return vertices_.size() - 1;
}

const Point3f & PolyMeshBuilder::GetVertex(GIndex index) const {
    ASSERT(index < vertices_.size());
    return vertices_[index];
}

void PolyMeshBuilder::MoveVertex(GIndex index, const Point3f &new_point) {
    KLOG('l', "PolyMeshBuilder moved vertex " << index << " to " << new_point);
    ASSERT(index < vertices_.size());
    vertices_[index] = new_point;
}

void PolyMeshBuilder::AddPolygon(const std::vector<GIndex> &indices) {
    KLOG('l', "PolyMeshBuilder added "
         << (indices.size() == 3U ? "TRI " :
             indices.size() == 4U ? "QUAD " : "POLY ")
         << Util::JoinItems(indices));
    ASSERT(indices.size() >= 3);
    PolyMesh::Border border;
    border.indices = indices;
    border.is_hole = false;
    borders_.push_back(border);
}

void PolyMeshBuilder::AddHole(const std::vector<GIndex> &indices) {
    KLOG('l', "PolyMeshBuilder added HOLE " << Util::JoinItems(indices));
    ASSERT(indices.size() >= 3);
    ASSERT(! borders_.empty());  // Must have outer border to add hole to.
    PolyMesh::Border border;
    border.indices = indices;
    border.is_hole = true;
    borders_.push_back(border);
}

void PolyMeshBuilder::BuildPolyMesh(PolyMesh &result_mesh) const {
    result_mesh.Set(vertices_, borders_);
}
