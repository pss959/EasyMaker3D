//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <vector>

#include "Math/TriMesh.h"
#include "Math/Types.h"

/// ModelMesh is a derived TriMesh that adds normals and teture coordinates..
///
/// \ingroup Math
struct ModelMesh : public TriMesh {
    /// Vertex normals
    std::vector<Vector3f> normals;

    /// Vertex texture coordinates.
    std::vector<Point2f>  tex_coords;

    /// Converts to a binary data string for compact output. The format is:
    ///   - *vertex_count*   : uint32
    ///   - *triangle_count* : uint32
    ///   - [*vertices*]     : 3 x float each
    ///   - [*normals*]      : 3 x float each
    ///   - [*tex_coords*]   : 2 x float each
    ///   - [*indices*]      : uint32 each
    ///
    /// The endianness of the host computer is used.
    Str ToBinaryString() const;

    /// Fills in this ModelMesh from the contents of a binary string. Returns
    /// false on error.
    bool FromBinaryString(const Str &str);
};
