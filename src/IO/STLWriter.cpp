#include "IO/STLWriter.h"

#include <fstream>

#include "Math/Linear.h"
#include "Math/MeshUtils.h"
#include "Models/Model.h"
#include "Selection.h"
#include "Util/Assert.h"

// ----------------------------------------------------------------------------
// XXXX Turn into class with derived STL text/binary versions
// ----------------------------------------------------------------------------

namespace {

static float Round_(float f) {
    const float kVecPrecision = .0001f;
    return RoundToPrecision(f, kVecPrecision);
}

// Safe negation (does not negate 0).
static float Negate_(float f) {
    return f ? -f : 0;
}

// XXXX Output points and vectors after converting from +Y-up to +Z-up.

static std::ostream & operator<<(std::ostream &out, const Point3f &p) {
    out << Round_(p[0]) << ' ' << Round_(p[2]) << ' ' << Round_(Negate_(p[1]));
    return out;
}

static std::ostream & operator<<(std::ostream &out, const Vector3f &v) {
    out << Round_(v[0]) << ' ' << Round_(v[2]) << ' ' << Round_(Negate_(v[1]));
    return out;
}

static void WriteMesh_(std::ostream &out, const TriMesh &mesh,
                       const UnitConversion &conv) {
    const size_t tri_count = mesh.GetTriangleCount();

    for (size_t i = 0; i < tri_count; ++i) {
        const Point3f &p0 = mesh.points[mesh.indices[3 * i + 0]];
        const Point3f &p1 = mesh.points[mesh.indices[3 * i + 1]];
        const Point3f &p2 = mesh.points[mesh.indices[3 * i + 2]];

        // XXXX Apply conversion.
        out << "  facet normal " << ComputeNormal(p0, p1, p2) << "\n";
        out << "    outer loop\n";
        out << "      vertex " << p0 << "\n";
        out << "      vertex " << p1 << "\n";
        out << "      vertex " << p2 << "\n";
        out << "    endloop";
        out << "  endfacet\n";
    }
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Public STL writing functions.
// ----------------------------------------------------------------------------

bool WriteSTLFile(const Selection &sel, const FilePath &path,
                  FileFormat format, const UnitConversion &conv) {
    std::ofstream out(path.ToNativeString());
    if (! out.is_open())
        return false;

    out << "solid MakerVR_Export\n";

    ASSERT(sel.HasAny());
    for (const auto &sel_path: sel.GetPaths()) {
        ASSERT(sel_path.GetModel());
        const auto &model = *sel_path.GetModel();

        // Convert the Model mesh to stage coordinates.
        const Matrix4f osm = sel_path.GetCoordConv().GetObjectToRootMatrix();
        WriteMesh_(out, TransformMesh(model.GetMesh(), osm), conv);
    }

    out << "endsolid MakerVR_Export\n";

    return true;
}
