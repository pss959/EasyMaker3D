//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Math/ModelMesh.h"

#include <ion/base/bufferbuilder.h>

#include "Util/Assert.h"

Str ModelMesh::ToBinaryString() const {
    const size_t pc = points.size();
    const size_t tc = GetTriangleCount();

    ASSERT(normals.size()    == pc);
    ASSERT(tex_coords.size() == pc);

    ion::base::BufferBuilder bb;
    bb.Append(static_cast<uint32>(pc));
    bb.Append(static_cast<uint32>(tc));
    bb.AppendArray(&points[0],     points.size());
    bb.AppendArray(&normals[0],    normals.size());
    bb.AppendArray(&tex_coords[0], tex_coords.size());
    bb.AppendArray(&indices[0],    indices.size());
    return bb.Build();
}

bool ModelMesh::FromBinaryString(const Str &str) {
    points.clear();
    indices.clear();
    normals.clear();
    tex_coords.clear();

    const uint8 *bp = reinterpret_cast<const uint8 *>(str.c_str());

    // Point and triangle counts.
    uint32 pc, tc;
    if (str.size() < sizeof(pc) + sizeof(tc))
        return false;
    pc = FromBinary<uint32>(bp);
    tc = FromBinary<uint32>(bp);

    // Make sure there is enough data.
    const size_t bytes_left  = str.size() - (sizeof(pc) + sizeof(tc));
    const size_t point_bytes = pc * 3 * sizeof(float);
    const size_t norm_bytes  = pc * 3 * sizeof(float);
    const size_t tex_bytes   = pc * 2 * sizeof(float);
    const size_t index_bytes = tc * 3 * sizeof(GIndex);
    if (bytes_left < point_bytes + norm_bytes + tex_bytes + index_bytes)
        return false;

    // Points.
    points.reserve(pc);
    for (size_t i = 0; i < pc; ++i) {
        const float x = FromBinary<float>(bp);
        const float y = FromBinary<float>(bp);
        const float z = FromBinary<float>(bp);
        points.push_back(Point3f(x, y, z));
    }

    // Normals.
    for (size_t i = 0; i < pc; ++i) {
        const float nx = FromBinary<float>(bp);
        const float ny = FromBinary<float>(bp);
        const float nz = FromBinary<float>(bp);
        normals.push_back(Vector3f(nx, ny, nz));
    }

    // Texture coordinates.
    for (size_t i = 0; i < pc; ++i) {
        const float u = FromBinary<float>(bp);
        const float v = FromBinary<float>(bp);
        tex_coords.push_back(Point2f(u, v));
    }

    // Indices.
    indices.reserve(3 * tc);
    for (size_t i = 0; i < 3 * tc; ++i)
        indices.push_back(FromBinary<GIndex>(bp));

    return true;
}
