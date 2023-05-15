#include "Math/MeshDividing.h"

#include <algorithm>
#include <limits>

#include "Math/CGALInternal.h"
#include "Math/Linear.h"
#include "Util/Assert.h"
#include "Util/KLog.h"

#include <CGAL/Polygon_mesh_processing/clip.h>

namespace {

// ----------------------------------------------------------------------------
// CGAL conversion functions.
// ----------------------------------------------------------------------------

/// Converts a Plane to a CGAL CPlane3.
static CPlane3 ToCPlane3(const Plane &plane) {
    const Vector4f coeffs = plane.GetCoefficients();
    return CPlane3(coeffs[0], coeffs[1], coeffs[2], coeffs[3]);
}

#if XXXX
// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

/// Determines the extent of the mesh in the given direction, returning the
/// projected min/max distances along the direction results as a Range1f.
static Range1f FindMeshExtents_(const TriMesh &mesh, const Vector3f &dir) {
    // Project each mesh point onto the vector and find the min/max distances.
    float min_dist =  std::numeric_limits<float>::max();
    float max_dist = -std::numeric_limits<float>::max();
    for (const auto &p: mesh.points) {
        const float dist = ion::math::Dot(Vector3f(p), dir);
        min_dist = std::min(min_dist, dist);
        max_dist = std::max(max_dist, dist);
    }
    ASSERT(min_dist < max_dist);
    return Range1f(min_dist, max_dist);
}
#endif

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

TriMesh ClipMesh(const TriMesh &mesh, const Plane &plane) {
    KLOG('C', "Clipping TriMesh with " << plane);

    CPolyhedron poly = TriMeshToCGALPolyhedron(mesh);
    CGAL::Polygon_mesh_processing::clip(poly, ToCPlane3(plane),
                                        CGAL::parameters::clip_volume(true));
    return CGALPolyhedronToTriMesh(poly);
}

TriMesh SplitMesh(const TriMesh &mesh, const Plane &plane) {
    KLOG('C', "Splitting TriMesh with " << plane);

    CPolyhedron poly = TriMeshToCGALPolyhedron(mesh);
    CGAL::Polygon_mesh_processing::split(poly, ToCPlane3(plane));
    return CGALPolyhedronToTriMesh(poly);
}

#if XXXX
SlicedMesh SliceMesh(const TriMesh &mesh, const Vector3f &dir,
                     size_t num_slices) {
    ASSERT(num_slices >= 1U);

    if (num_slices == 1U) {
        SlicedMesh sliced_mesh;
        sliced_mesh.mesh  = mesh;
        sliced_mesh.dir   = dir;
        sliced_mesh.range = FindMeshExtents_(mesh, dir);
        return sliced_mesh;
    }
    else {
        // Construct a vector of fractions.
        std::vector<float> fractions(num_slices - 1, 0.f);
        for (size_t i = 1; i < num_slices; ++i)
            fractions[i - 1] = static_cast<float>(i) / num_slices;
        return SliceMesh(mesh, dir, fractions);
    }
}

SlicedMesh SliceMesh(const TriMesh &mesh, const Vector3f &dir,
                     const std::vector<float> &fractions) {
    ASSERT(IsValidVector(dir));
    ASSERT(! fractions.empty());

    // Determine the extent of the mesh in the given direction.
    const auto range = FindMeshExtents_(mesh, dir);

    CPolyhedron poly = TriMeshToCGALPolyhedron(mesh);

    // Create planes perpendicular to the direction and split into slices.
    for (const auto frac: fractions) {
        const float dist = Lerp(frac, range.GetMinPoint(), range.GetMaxPoint());
        CGAL::Polygon_mesh_processing::split(poly, ToCPlane3(Plane(dist, dir)));
    }

    SlicedMesh sliced_mesh;
    sliced_mesh.mesh  = CGALPolyhedronToTriMesh(poly);
    sliced_mesh.dir   = dir;
    sliced_mesh.range = range;
    return sliced_mesh;
}
#endif

