#include "Math/SlicedMesh.h" // XXXX

#include <algorithm>
#include <cmath>
#include <limits>

#include <ion/math/vectorutils.h>

#include "Math/Linear.h"
#include "Math/MeshUtils.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/String.h" // XXXX

namespace {

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

#if XXXX
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

static Range1f FindMeshYExtents_(const TriMesh &mesh) {
    float min_y =  std::numeric_limits<float>::max();
    float max_y = -std::numeric_limits<float>::max();
    for (const auto &p: mesh.points) {
        min_y = std::min(min_y, p[1]);
        max_y = std::max(max_y, p[1]);
    }
    ASSERT(min_y < max_y);
    return Range1f(min_y, max_y);
}

/// Returns index into ys if equal, index+1/2 if between value and next.
static float FindBucket_(float y, const std::vector<float> &ys) {
    // XXXX binary search.
    auto lower = std::lower_bound(ys.begin(), ys.end(), y);
    ASSERT(lower != ys.end());
    const size_t index = std::distance(ys.begin(), lower);
    std::cerr << "XXXX Found " << y << " at "
              << index << " (" << ys[index] << ")\n";

    return y == ys[index] ? index : index + .5f;
}

/// XXXX
static void SplitTri_(const TriMesh &mesh, GIndex i0, GIndex i1, GIndex i2,
                      float y, TriMesh &new_mesh) {
    const auto &p0 = mesh.points[i0];
    const auto &p1 = mesh.points[i1];
    const auto &p2 = mesh.points[i2];

    std::cerr << "XXXX Splitting tri <" << p0 << ", " << p1
              << ", " << p2 << "> at Y=" << y << "\n";

    // p0 is the point on the other side of the y value from p1 and p2. Find
    // the two new points on the edges from p0 to p1 and p0 to p2.
    const Point3f np1 = Lerp((p0[1] - y) / (p0[1] - p1[1]), p0, p1);
    const Point3f np2 = Lerp((p0[1] - y) / (p0[1] - p2[1]), p0, p2);

    // Add the new points.
    new_mesh.points.push_back(np1);
    new_mesh.points.push_back(np2);
    const GIndex ni1 = new_mesh.points.size() - 2;
    const GIndex ni2 = new_mesh.points.size() - 1;

    // The split creates a triangle and a trapezoid, which is split into 2
    // triangles.
    // Triangle:
    new_mesh.indices.push_back(i0);
    new_mesh.indices.push_back(ni1);
    new_mesh.indices.push_back(ni2);
    // Trapezoid:
    new_mesh.indices.push_back(ni1);
    new_mesh.indices.push_back(i1);
    new_mesh.indices.push_back(i2);
    new_mesh.indices.push_back(ni1);
    new_mesh.indices.push_back(i2);
    new_mesh.indices.push_back(ni2);
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

SlicedMesh NSliceMesh(const TriMesh &mesh, const Vector3f &dir,
                      const std::vector<float> &fractions) {
    std::cerr << "XXXX ---- Slicing mesh " << mesh.ToString()
              << " by " << Util::JoinItems(fractions) << "\n";

    // XXXX Assume direction is Y axis. Rotate if not...

    ASSERT(IsValidVector(dir));
    ASSERT(! fractions.empty());

    // Determine the extent of the mesh in the given direction.
    const auto range = FindMeshYExtents_(mesh);

    // Convert the fractions into Y values.
    std::vector<float> ys = Util::ConvertVector<float, float>(
        fractions, [&](const float &f){ return Lerp(f, range.GetMinPoint(),
                                                    range.GetMaxPoint()); });

    // Sort the distances and add the min/max at either end. This makes the
    // bucket search have fewer special cases.
    std::sort(ys.begin(), ys.end());
    ys.insert(ys.begin(), range.GetMinPoint());
    ys.push_back(range.GetMaxPoint());
    std::cerr << "XXXX Ys = " << Util::JoinItems(ys) << "\n";

    // Sort the points of the mesh into buckets based on the distances. For
    // each point, store the bucket index.
    const std::vector<float> buckets = Util::ConvertVector<float, Point3f>(
        mesh.points, [&](const Point3f &p){ return FindBucket_(p[1], ys); });

    // This TriMesh will contain the split triangles.
    TriMesh new_mesh;
    new_mesh.points = mesh.points;
    new_mesh.indices.reserve(2 * mesh.indices.size());

    // For each mesh triangle, determine if it crosses two or more buckets.
    const size_t tri_count = mesh.GetTriangleCount();
    for (size_t i = 0; i < tri_count; ++i) {
        const auto i0 = mesh.indices[3 * i + 0];
        const auto i1 = mesh.indices[3 * i + 1];
        const auto i2 = mesh.indices[3 * i + 2];

        const auto b0 = buckets[i0];
        const auto b1 = buckets[i1];
        const auto b2 = buckets[i2];

        // If the minimum is in bucket N or N+.5, then the maximum has to be in
        // bucket >= N+1.5 for there to be a bucket crossing. In general, there
        // are C crossings if the maximum is at N+C+.5 or N+C+1 (which is
        // equivalent to the ceiling of the maximum at N+C+1).
        const auto min = std::min(b0, std::min(b1, b2));
        const auto max = std::max(b0, std::max(b1, b2));
        const auto diff = std::ceil(max) - std::floor(min);
        const int num_buckets = static_cast<int>(diff);

        std::cerr << "XXXX Tri " << i << ":"
                  << " B=(" << b0 << "/" << b1 << "/" << b2 << ")"
                  << " MN=" << min
                  << " MX=" << max
                  << " D=" << diff
                  << " NB=" << num_buckets << "\n";

        // Copy the triangle if it does not need to be split.
        if (num_buckets <= 1) {
            new_mesh.indices.push_back(i0);
            new_mesh.indices.push_back(i1);
            new_mesh.indices.push_back(i2);
        }

        // Split if it crosses 2 or more buckets.
        else {

            // Need to split the triangle. If the triangle crosses only 1 slice
            // plane, this is easier.
            if (num_buckets == 2) {
                const float slice_y = ys[min + 1];
                // One of the vertices is in a different bucket.
                const int b0i = static_cast<int>(b0);
                const int b1i = static_cast<int>(b1);
                const int b2i = static_cast<int>(b2);
                if (b0i == b1i) {
                    std::cerr << "XXXX   2/" << b2i << " is different bucket\n";
                    SplitTri_(mesh, i2, i0, i1, slice_y, new_mesh);
                }
                else if (b0i == b2i) {
                    std::cerr << "XXXX   1/" << b1i << " is different bucket\n";
                    SplitTri_(mesh, i1, i2, i0, slice_y, new_mesh);
                }
                else {
                    ASSERT(b1i == b2i);
                    std::cerr << "XXXX   0/" << b0i << " is different bucket\n";
                    SplitTri_(mesh, i0, i1, i2, slice_y, new_mesh);
                }
            }
            else {
                std::cerr << "XXXX Need to handle > 2 buckets\n";
            }
        }
    }

    // Remove redundant vertices and any other issues.
    CleanMesh(new_mesh);

    std::cerr << "XXXX ---- Sliced mesh: " << new_mesh.ToString() << "\n";

    SlicedMesh sliced_mesh;
    sliced_mesh.mesh  = new_mesh;
    sliced_mesh.dir   = dir;
    sliced_mesh.range = range;
    return sliced_mesh;
}
