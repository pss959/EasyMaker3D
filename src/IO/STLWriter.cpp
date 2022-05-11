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

static void WriteMeshB_(std::ostream &out, const TriMesh &mesh,
                        const UnitConversion &conv) {
    const size_t tri_count = mesh.GetTriangleCount();

    for (size_t i = 0; i < tri_count; ++i) {
        Point3f p0 = mesh.points[mesh.indices[3 * i + 0]];
        Point3f p1 = mesh.points[mesh.indices[3 * i + 1]];
        Point3f p2 = mesh.points[mesh.indices[3 * i + 2]];

        // Convert coordinate systems.
        p0.Set(p0[0], p0[2], Negate_(p0[1]));
        p1.Set(p1[0], p1[2], Negate_(p1[1]));
        p2.Set(p2[0], p2[2], Negate_(p2[1]));

        // Write the normal.
        const Vector3f normal = ComputeNormal(p0, p1, p2);
        out.write(reinterpret_cast<const char *>(&normal), sizeof(normal));

        // Write the three vertices.
        out.write(reinterpret_cast<const char *>(&p0), sizeof(p0));
        out.write(reinterpret_cast<const char *>(&p1), sizeof(p1));
        out.write(reinterpret_cast<const char *>(&p2), sizeof(p2));

        // Attributes.
        const uint16 attributes = 0;
        out.write(reinterpret_cast<const char *>(&attributes),
                  sizeof(attributes));

        // XXXX Apply conversion.
    }
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Public STL writing functions.
// ----------------------------------------------------------------------------

bool WriteSTLFile(const Selection &sel, const FilePath &path,
                  FileFormat format, const UnitConversion &conv) {
    ASSERT(format != FileFormat::kUnknown);
    ASSERT(sel.HasAny());

    if (format == FileFormat::kTextSTL) {
        std::ofstream out(path.ToNativeString());
        if (! out.is_open())
            return false;

        out << "solid MakerVR_Export\n";

        for (const auto &sel_path: sel.GetPaths()) {
            ASSERT(sel_path.GetModel());
            const auto &model = *sel_path.GetModel();

            // Convert the Model mesh to stage coordinates.
            const Matrix4f osm = sel_path.GetCoordConv().GetObjectToRootMatrix();
            WriteMesh_(out, TransformMesh(model.GetMesh(), osm), conv);
        }

        out << "endsolid MakerVR_Export\n";
    }
    else if (format == FileFormat::kBinarySTL) {
        std::ofstream out(path.ToNativeString(), std::ios::binary);
        if (! out.is_open())
            return false;

        // Create an 80-byte header.
        std::string header = "MakerVR_Export";
        header += std::string(80 - header.size(), ' ');
        out.write(&header[0], 80);

        // Write the total number of triangles as 4 bytes.
        uint32 tri_count = 0;
        for (const auto &sel_path: sel.GetPaths()) {
            ASSERT(sel_path.GetModel());
            tri_count += sel_path.GetModel()->GetTriangleCount();
        }
        out.write(reinterpret_cast<const char *>(&tri_count),
                  sizeof(tri_count));

        // Write each Model.
        for (const auto &sel_path: sel.GetPaths()) {
            const auto &model = *sel_path.GetModel();

            // Convert the Model mesh to stage coordinates.
            const Matrix4f osm = sel_path.GetCoordConv().GetObjectToRootMatrix();
            WriteMeshB_(out, TransformMesh(model.GetMesh(), osm), conv);
        }
    }

    return true;
}
