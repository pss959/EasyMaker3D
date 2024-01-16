#include "Math/TriMesh.h"

#include <ion/base/bufferbuilder.h>

Str TriMesh::ToBinaryString() const {
    const size_t pc = points.size();
    const size_t tc = GetTriangleCount();

    ion::base::BufferBuilder bb;
    bb.Append(static_cast<uint32>(pc));
    bb.Append(static_cast<uint32>(tc));
    bb.AppendArray(&points[0],  points.size());
    bb.AppendArray(&indices[0], indices.size());
    return bb.Build();
}

bool TriMesh::FromBinaryString(const Str &str) {
    points.clear();
    indices.clear();

    const uint8 *bp = reinterpret_cast<const uint8 *>(str.c_str());

    // Point and triangle counts.
    uint32 pc, tc;
    if (str.size() < sizeof(pc) + sizeof(tc))
        return false;
    pc = FromBinary<uint32>(bp);
    tc = FromBinary<uint32>(bp);

    // Make sure there is enough data for points and indices.
    const size_t bytes_left      = str.size() - (sizeof(pc) + sizeof(tc));
    const size_t point_bytes = pc * 3 * sizeof(float);
    const size_t index_bytes = tc * 3 * sizeof(GIndex);
    if (bytes_left < point_bytes + index_bytes)
        return false;

    // Points.
    points.reserve(pc);
    for (size_t i = 0; i < pc; ++i) {
        const float x = FromBinary<float>(bp);
        const float y = FromBinary<float>(bp);
        const float z = FromBinary<float>(bp);
        points.push_back(Point3f(x, y, z));
    }
    // Indices.
    indices.reserve(3 * tc);
    for (size_t i = 0; i < 3 * tc; ++i)
        indices.push_back(FromBinary<GIndex>(bp));

    return true;
}

// LCOV_EXCL_START [debug only]
Str TriMesh::ToString() const {
    const size_t pc = points.size();
    const size_t tc = GetTriangleCount();
    Str s = "TriMesh with " + ToStr(pc) + " points and " +
        ToStr(tc) + " triangles:\n";
    for (size_t i = 0; i < pc; ++i)
        s += "   [" + ToStr(i) + "] " + ToStr(points[i]) + "\n";

    auto i2s = [&](size_t index){ return ToStr(indices[index]); };
    for (size_t i = 0; i < tc; ++i)
        s += "   TRI [" +
            i2s(3 * i + 0) + " " +
            i2s(3 * i + 1) + " " +
            i2s(3 * i + 2) + "]\n";
    return s;
}
// LCOV_EXCL_STOP
