#include "Math/MeshUtils.h"

namespace Math {

void WriteMeshAsOFF(const SG::TriMesh &mesh, const std::string &description,
                    std::ostream &out) {
    out << "OFF\n";
    out << "# " << description << ":\n";
    const size_t tri_count = mesh.indices.size() / 3U;
    out << mesh.points.size() << ' ' << tri_count << " 0\n";
    for (size_t i = 0; i < mesh.points.size(); ++i) {
        const SG::Point3f &p = mesh.points[i];
        out << p[0] << ' ' << p[1] << ' ' << p[2] << "\n";
    }
    for (size_t i = 0; i < tri_count; ++i) {
        const int i0 = mesh.indices[3 * i + 0];
        const int i1 = mesh.indices[3 * i + 1];
        const int i2 = mesh.indices[3 * i + 2];
        out << "3 " << i0 << ' ' << i1 << ' ' << i2 << "\n";
    }
}

}  // namespace Util
