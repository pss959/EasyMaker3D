#include "IO/STLWriter.h"

#include <fstream>

#include "Math/Linear.h"
#include "Math/MeshUtils.h"
#include "Models/Model.h"
#include "Selection.h"
#include "UnitConversion.h"
#include "Util/Assert.h"

// ----------------------------------------------------------------------------
// Helper classes.
// ----------------------------------------------------------------------------

namespace {

/// Abstract base class for an STL writer. This contains convenience functions
/// that allow derived classes to produce consistent results.
class STLWriter_ {
  public:
    /// Implements STL writing. Returns false on error.
    bool Write(const Selection &sel, const FilePath &path,
               const UnitConversion &conv);

  protected:
    /// Returns mode flags for opening the file to write to.
    virtual std::ios::openmode GetMode() const = 0;

    /// Writes the file header to the stream.
    virtual void WriteHeader(std::ostream &out) = 0;

    /// Writes a mesh (processed to the correct coordinates) to the stream.
    virtual void WriteMesh(std::ostream &out, const TriMesh &mesh) = 0;

    /// Writes the file footer to the stream.
    virtual void WriteFooter(std::ostream &out) = 0;

    /// Returns the normal to the triangle defined by the 3 given points,
    /// cleaning up the result.
    static Vector3f GetNormal(const Point3f &p0,
                              const Point3f &p1,
                              const Point3f &p2) {
        Vector3f normal = ComputeNormal(p0, p1, p2);
        for (int i = 0; i < 3; ++i) {
            if (normal[i] == -0)
                normal[i] = 0;
        }
        return normal;
    }

  private:
    std::ofstream out_;

    /// Processes the mesh for the Model specified by the given SelPath to get
    /// a mesh that can be written as STL: transforms vertices to stage
    /// coordinates, applies the UnitConversion factor, applies rounding, and
    /// converts the results to the STL coordinate system (Z-up).
    static TriMesh ProcessModelMesh_(const SelPath &sel_path,
                                     const UnitConversion &conv);
};

bool STLWriter_::Write(const Selection &sel, const FilePath &path,
                       const UnitConversion &conv) {
    // Open the stream for writing.
    std::ofstream out(path.ToNativeString(), GetMode());
    if (! out.is_open())
        return false;

    WriteHeader(out);

    for (const auto &sel_path: sel.GetPaths())
        WriteMesh(out, ProcessModelMesh_(sel_path, conv));

    WriteFooter(out);

    return true;
}

TriMesh STLWriter_::ProcessModelMesh_(const SelPath &sel_path,
                                      const UnitConversion &conv) {
    ASSERT(sel_path.GetModel());

    // Apply the matrix to the mesh to convert to stage coordinates.
    const Matrix4f osm = sel_path.GetCoordConv().GetObjectToRootMatrix();
    TriMesh mesh = TransformMesh(sel_path.GetModel()->GetMesh(), osm);

    const float conv_factor   = conv.GetFactor();
    const float kPrecision = .0001f;

    // Process each vertex.
    for (auto &point: mesh.points) {
        // Apply the conversion factor.
        point *= conv_factor;

        // Round.
        for (int dim = 0; dim < 3; ++dim)
            point[dim] = RoundToPrecision(point[dim], kPrecision);

        // Convert to STL coordinates (from Y-up to Z-up). Be careful when
        // negating not to create -0.
        point.Set(point[0], point[2], point[1] ? -point[1] : 0);
    }

    return mesh;
}

// XXXX
static std::ostream & operator<<(std::ostream &out, const Vector3f &v) {
    out << v[0] << ' ' << v[1] << ' ' << v[2];
    return out;
}
static std::ostream & operator<<(std::ostream &out, const Point3f &p) {
    out << p[0] << ' ' << p[1] << ' ' << p[2];
    return out;
}

/// Text version.
class STLTextWriter_ : public STLWriter_ {
  protected:
    virtual std::ios::openmode GetMode() const override {
        return std::ios::out;
    }
    virtual void WriteHeader(std::ostream &out) override {
        out << "solid MakerVR_Export\n";
    }
    virtual void WriteMesh(std::ostream &out, const TriMesh &mesh) override;
    virtual void WriteFooter(std::ostream &out) override {
        out << "endsolid MakerVR_Export\n";
    }
  private:
    /// Writes a point or vector.
    template <typename T>
    std::ostream & WriteV_(std::ostream &out, const T &v) {
        out << v[0] << ' ' << v[1] << ' ' << v[2];
        return out;
    }
};

void STLTextWriter_::WriteMesh(std::ostream &out, const TriMesh &mesh) {
    const size_t tri_count = mesh.GetTriangleCount();
    for (size_t i = 0; i < tri_count; ++i) {
        const Point3f &p0 = mesh.points[mesh.indices[3 * i + 0]];
        const Point3f &p1 = mesh.points[mesh.indices[3 * i + 1]];
        const Point3f &p2 = mesh.points[mesh.indices[3 * i + 2]];
        out << "  facet normal " << GetNormal(p0, p1, p2) << "\n";
        out << "    outer loop\n";
        out << "      vertex " << p0 << "\n";
        out << "      vertex " << p1 << "\n";
        out << "      vertex " << p2 << "\n";
        out << "    endloop";
        out << "  endfacet\n";
    }
}

/// Binary version.
class STLBinaryWriter_ : public STLWriter_ {
};

// Safe negation (does not negate 0).
static float Negate_(float f) {
    return f ? -f : 0;
}

// XXXX Output points and vectors after converting from +Y-up to +Z-up.

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
        STLTextWriter_ writer;
        return writer.Write(sel, path, conv);
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
