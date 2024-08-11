//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "IO/OFFWriter.h"

#include <fstream>
#include <numeric>

#include "Math/Color.h"
#include "Math/TriMesh.h"
#include "Math/Types.h"
#include "Util/Assert.h"
#include "Util/FilePath.h"

// ----------------------------------------------------------------------------
// Helper functions
// ----------------------------------------------------------------------------

namespace {

static void WritePoint_(const Point3f &p, std::ostream &out) {
    out << p[0] << " " << p[1] << " " << p[2] << "\n";
}


static void WriteColor_(const Color &c, std::ostream &out) {
    // OFF colors are integers from 0 to 255.
    auto to_255 = [](float f){ return static_cast<int>(255 * f); };
    out << " " << to_255(c[0]) << " " << to_255(c[1]) << " " << to_255(c[2]);
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

bool WriteOFFFile(const std::vector<TriMesh> &meshes,
                  const std::vector<Color> &colors, const FilePath &path) {
    ASSERT(! meshes.empty());

    std::ofstream out(path.ToNativeString());
    if (! out.is_open())
        return false;

    // Add up vertices and face counts.
    const size_t vertex_count =
        std::accumulate(meshes.begin(), meshes.end(),
                        0, [](size_t sum, const TriMesh &m){
                            return sum + m.points.size(); });
    const size_t face_count =
        std::accumulate(meshes.begin(), meshes.end(),
                        0, [](size_t sum, const TriMesh &m){
                            return sum + m.GetTriangleCount(); });

    // Header and vertex/face/edge count. (Edge count is ignored.)
    out << "OFF\n";
    out << vertex_count << " " << face_count << " 0\n";

    // All vertices.
    for (const auto &mesh: meshes) {
        for (const auto &p: mesh.points)
            WritePoint_(p, out);
    }

    // All triangular faces with optional colors.
    size_t cur_color = 0;
    GIndex cur_index_offset = 0;
    for (const auto &mesh: meshes) {
        const size_t tri_count = mesh.GetTriangleCount();
        for (size_t i = 0; i < tri_count; ++i) {
            const auto i0 = cur_index_offset + mesh.indices[3 * i + 0];
            const auto i1 = cur_index_offset + mesh.indices[3 * i + 1];
            const auto i2 = cur_index_offset + mesh.indices[3 * i + 2];
            out << "3 " << i0 << " " << i1 << " " << i2;
            if (! colors.empty()) {
                WriteColor_(colors[cur_color], out);
                cur_color = (cur_color + 1) % colors.size();
            }
            out << "\n";
        }
        cur_index_offset += mesh.points.size();
    }

    return true;
}
