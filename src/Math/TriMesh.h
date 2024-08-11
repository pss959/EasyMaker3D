//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <vector>

#include "Math/Types.h"

/// A TriMesh struct represents a 3D triangle mesh.
///
/// \ingroup Math
struct TriMesh {
    /// A point on the mesh resulting from a Ray intersection.
    struct Hit {
        Point3f  point;        ///< Point of intersection.
        Vector3f normal;       ///< Normal to the triangle.
        Vector3i indices;      ///< Indices of triangle the point is on.
        Vector3f barycentric;  ///< Barycentric coordinates at the point.
    };

    /// Vertex points forming the mesh, in no particular order.
    std::vector<Point3f> points;

    /// Point indices forming triangles, 3 per triangle.
    std::vector<GIndex>  indices;

    /// Convenience that returns the number of triangles.
    size_t GetTriangleCount() const { return indices.size() / 3; }

    /// Converts to a (potentially very long) string to help with debugging.
    Str ToString() const;

    /// Converts to a binary data string for compact output. The format is:
    ///   - *vertex_count*   : uint32
    ///   - *triangle_count* : uint32
    ///   - [*vertices*]     : 3 x float each
    ///   - [*indices*]      : uint32 each
    ///
    /// The endianness of the host computer is used.
    Str ToBinaryString() const;

    /// Fills in this TriMesh from the contents of a binary string. Returns
    /// false on error.
    bool FromBinaryString(const Str &str);
};
