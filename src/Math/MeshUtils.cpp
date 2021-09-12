#include "Math/MeshUtils.h"

#include <functional>

#include <ion/math/transformutils.h>

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

//! Creates a new mesh by modifying each point in the given mesh using the
//! given function. If change_orientation is true, this also changes the
//! orientation of the new mesh's triangles.
static TriMesh ModifyVertices_(
    const TriMesh &mesh, const std::function<Point3f(const Point3f &)> &func,
    bool change_orientation = false) {
    TriMesh new_mesh;

    const size_t pt_count  = mesh.points.size();
    new_mesh.points.resize(pt_count);
    for (size_t i = 0; i < pt_count; ++i)
        new_mesh.points[i] = func(mesh.points[i]);

    // Change the orientation of all triangles if requested.
    if (change_orientation) {
        const size_t tri_count = mesh.indices.size() / 3;
        new_mesh.indices.resize(3 * tri_count);
        for (size_t i = 0; i < tri_count; ++i) {
            new_mesh.indices[3 * i + 0] = mesh.indices[3 * i + 2];
            new_mesh.indices[3 * i + 1] = mesh.indices[3 * i + 1];
            new_mesh.indices[3 * i + 2] = mesh.indices[3 * i + 0];
        }
    }
    else {
        new_mesh.indices = mesh.indices;
    }
    return new_mesh;
}

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

TriMesh TransformMesh(const TriMesh &mesh, const Matrix4f &m) {
    return ModifyVertices_(mesh, [m](const Point3f &p){ return m * p; });
}

TriMesh MirrorMesh(const TriMesh &mesh, const Plane &plane) {
    return ModifyVertices_(mesh, [plane](const Point3f &p){
        return plane.MirrorPoint(p); }, true);
}

Vector3f CenterMesh(TriMesh &mesh) {
    const Bounds bounds = ComputeMeshBounds(mesh);
    const Vector3f offset = Vector3f(-bounds.GetCenter());
    if (offset != Vector3f::Zero()) {
        for (auto &p: mesh.points)
            p += offset;
    }
    return offset;
}

void WriteMeshAsOFF(const TriMesh &mesh, const std::string &description,
                    std::ostream &out) {
    out << "OFF\n";
    out << "# " << description << ":\n";
    const size_t tri_count = mesh.indices.size() / 3U;
    out << mesh.points.size() << ' ' << tri_count << " 0\n";
    for (size_t i = 0; i < mesh.points.size(); ++i) {
        const Point3f &p = mesh.points[i];
        out << p[0] << ' ' << p[1] << ' ' << p[2] << "\n";
    }
    for (size_t i = 0; i < tri_count; ++i) {
        const int i0 = mesh.indices[3 * i + 0];
        const int i1 = mesh.indices[3 * i + 1];
        const int i2 = mesh.indices[3 * i + 2];
        out << "3 " << i0 << ' ' << i1 << ' ' << i2 << "\n";
    }
}

Bounds ComputeMeshBounds(const TriMesh &mesh) {
    Bounds bounds;
    for (const Point3f &pt: mesh.points)
        bounds.ExtendByPoint(pt);
    return bounds;
}
