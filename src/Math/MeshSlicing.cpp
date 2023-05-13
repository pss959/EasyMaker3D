#include "Math/SlicedMesh.h" // XXXX

#include <algorithm>
#include <cmath>
#include <limits>

#include <ion/math/vectorutils.h>

#include "Math/Linear.h"
#include "Math/MeshUtils.h"
#include "Util/Assert.h"
#include "Util/Enum.h"
#include "Util/General.h"
#include "Util/String.h" // XXXX

namespace {

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

/// Finds the min and max coordinates in the given dimension among all points
/// in the mesh, returning the result as a Range1f.
static Range1f FindMeshExtents_(const TriMesh &mesh, int dim) {
    ASSERT(dim >= 0 && dim <= 2);
    float min =  std::numeric_limits<float>::max();
    float max = -std::numeric_limits<float>::max();
    for (const auto &p: mesh.points) {
        min = std::min(min, p[dim]);
        max = std::max(max, p[dim]);
    }
    ASSERT(min < max);
    return Range1f(min, max);
}

/// Finds the bucket in which the given value lies relative to the given vector
/// of values. If the value is the same as one of the values, this returns its
/// index. Otherwise, it returns the number halfway between the indices of the
/// two values surrounding it.
static float FindBucket_(float val, const std::vector<float> &vals) {
    // Do a binary search.
    auto lower = std::lower_bound(vals.begin(), vals.end(), val);
    ASSERT(lower != vals.end());
    const size_t index = std::distance(vals.begin(), lower);
    return val == vals[index] ? index : index + .5f;
}

/// Splits the TriMesh triangle with the given indices across \p slice_val in
/// dimension \p dim, adding the resulting points and triangles to \p new_mesh.
static void SplitTri_(const TriMesh &mesh, GIndex i0, GIndex i1, GIndex i2,
                      int dim, float slice_val, TriMesh &new_mesh) {
    ASSERT(dim >= 0 && dim <= 2);
    const auto &p0 = mesh.points[i0];
    const auto &p1 = mesh.points[i1];
    const auto &p2 = mesh.points[i2];

    std::cerr << "XXXX Splitting tri <" << i0 << ", " << i1
              << ", " << i2 << "> at VAL=" << slice_val << "\n";

    // p0 is the point on the other side of slice_val from p1 and p2. Find the
    // two new points on the edges from p0 to p1 and p0 to p2.
    const Point3f np1 = Lerp((p0[dim] - slice_val) / (p0[dim] - p1[dim]),
                             p0, p1);
    const Point3f np2 = Lerp((p0[dim] - slice_val) / (p0[dim] - p2[dim]),
                             p0, p2);

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

/// Splits the TriMesh triangle with the given indices across all \p slice_vals
/// in dimension \p dim, adding the resulting points and triangles to \p
/// new_mesh. The values in \p slice_vals must be sorted in increasing order.
static void SplitTri_(const TriMesh &mesh, GIndex i0, GIndex i1, GIndex i2,
                      int dim, const std::vector<float> &slice_vals,
                      TriMesh &new_mesh) {
    ASSERT(dim >= 0 && dim <= 2);
    ASSERT(slice_vals.size() >= 2U);
    const auto &p0 = mesh.points[i0];
    const auto &p1 = mesh.points[i1];
    const auto &p2 = mesh.points[i2];

    ASSERT(p0[dim] < slice_vals.front());
    ASSERT(p1[dim] > slice_vals.back() || p2[dim] > slice_vals.back());

    std::cerr << "XXXX Splitting tri <" << i0 << ", " << i1
              << ", " << i2 << "> at "
              << slice_vals.size() << ": "
              << Util::JoinItems(slice_vals) << "\n";

    const auto ADDTRI = [&](GIndex a, GIndex b, GIndex c, size_t i, char cs){
        new_mesh.indices.push_back(a);
        new_mesh.indices.push_back(b);
        new_mesh.indices.push_back(c);
        std::cerr << "XXXX Added Tri " << (new_mesh.GetTriangleCount() - 1)
                  << ": " << a << " / " << b << " / " << c
                  << " from trap " << i << " in case " << cs << "\n";
    };

    // p0 is below the first slicing value. If either other point is, then
    // create a trapezoid.
    float y = slice_vals[0];
    if (p1[dim] < y) {  // Case A.
        std::cerr << "XXXX A ----------------------------i = " << 0 << "\n";
        const Point3f np0 = Lerp((p0[dim] - y) / (p0[dim] - p2[dim]), p0, p2);
        const Point3f np1 = Lerp((p1[dim] - y) / (p1[dim] - p2[dim]), p1, p2);

        // Add the new points.
        new_mesh.points.push_back(np0);
        new_mesh.points.push_back(np1);
        GIndex ni0 = new_mesh.points.size() - 2;
        GIndex ni1 = new_mesh.points.size() - 1;

        // Trapezoid:
        ADDTRI(i0,  i1, ni0, 0, 'A');
        ADDTRI(ni0, i1, ni1, 0, 'A');

        // Continue to following trapezoids.
        for (size_t i = 1; i < slice_vals.size(); ++i) {
            std::cerr << "XXXX A ----------------------------i = " << i << "\n";
            float y = slice_vals[i];
            const Point3f np0 = Lerp((p0[dim] - y) / (p0[dim] - p2[dim]), p0, p2);
            const Point3f np1 = Lerp((p1[dim] - y) / (p1[dim] - p2[dim]), p1, p2);

            const GIndex prev0 = ni0;
            const GIndex prev1 = ni1;

            new_mesh.points.push_back(np0);
            new_mesh.points.push_back(np1);
            ni0 = new_mesh.points.size() - 2;
            ni1 = new_mesh.points.size() - 1;

            // Trapezoid.
            ADDTRI(prev0, prev1, ni0, i, 'A');
            ADDTRI(ni0, prev1, ni1, i, 'A');
        }

        // Triangle at the top.
        ADDTRI(ni0, ni1, i2, slice_vals.size(), 'A');
    }
    else if (p2[dim] < y) {  // Case B.
        std::cerr << "XXXX B ----------------------------i = " << 0 << "\n";
        const Point3f np2 = Lerp((p2[dim] - y) / (p2[dim] - p1[dim]), p2, p1);
        const Point3f np0 = Lerp((p0[dim] - y) / (p0[dim] - p1[dim]), p0, p1);

        // Add the new points.
        new_mesh.points.push_back(np2);
        new_mesh.points.push_back(np0);
        GIndex ni2 = new_mesh.points.size() - 2;
        GIndex ni0 = new_mesh.points.size() - 1;

        // Trapezoid:
        ADDTRI(i2,  i0, ni2, 0, 'B');
        ADDTRI(ni2, i0, ni0, 0, 'B');

        // Continue to following trapezoids.
        for (size_t i = 1; i < slice_vals.size(); ++i) {
            std::cerr << "XXXX B ----------------------------i = " << i << "\n";
            float y = slice_vals[i];

            const Point3f np2 = Lerp((p2[dim] - y) / (p2[dim] - p1[dim]), p2, p1);
            const Point3f np0 = Lerp((p0[dim] - y) / (p0[dim] - p1[dim]), p0, p1);

            const GIndex prev2 = ni2;
            const GIndex prev0 = ni0;

            new_mesh.points.push_back(np2);
            new_mesh.points.push_back(np0);
            ni2 = new_mesh.points.size() - 2;
            ni0 = new_mesh.points.size() - 1;

            // Trapezoid.
            ADDTRI(prev2, prev0, ni2, i, 'B');
            ADDTRI(ni2, prev0, ni0, i, 'B');
        }

        // Triangle at the top.
        ADDTRI(ni2, ni0, i1, slice_vals.size(), 'B');
    }
    else {  // Case C.
        std::cerr << "XXXX C ----------------------------i = " << 0 << "\n";
        // Otherwise, only p0 is below y. Add a clipped triangle.
        const Point3f np1 = Lerp((p0[dim] - y) / (p0[dim] - p1[dim]), p0, p1);
        const Point3f np2 = Lerp((p0[dim] - y) / (p0[dim] - p2[dim]), p0, p2);

        // Add the new points.
        new_mesh.points.push_back(np1);
        new_mesh.points.push_back(np2);
        GIndex ni1 = new_mesh.points.size() - 2;
        GIndex ni2 = new_mesh.points.size() - 1;

        // Triangle:
        ADDTRI(i0, ni1, ni2, 0, 'C');

        // Continue to following trapezoids.
        for (size_t i = 1; i < slice_vals.size(); ++i) {
            std::cerr << "XXXX C ----------------------------i = " << i << "\n";
            float y = slice_vals[i];

            const Point3f np1 = Lerp((p0[dim] - y) / (p0[dim] - p1[dim]), p0, p1);
            const Point3f np2 = Lerp((p0[dim] - y) / (p0[dim] - p2[dim]), p0, p2);

            const GIndex prev1 = ni1;
            const GIndex prev2 = ni2;

            new_mesh.points.push_back(np1);
            new_mesh.points.push_back(np2);
            ni1 = new_mesh.points.size() - 2;
            ni2 = new_mesh.points.size() - 1;

            // Trapezoid.
            ADDTRI(prev1, ni1, prev2, i, 'C');
            ADDTRI(prev2, ni1, ni2, i, 'C');
        }

        // Trapezoid at the top.
        ADDTRI(ni1, i1, ni2, slice_vals.size(), 'C');
        ADDTRI(ni2, i1, i2,  slice_vals.size(), 'C');
    }
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

SlicedMesh SliceMesh(const TriMesh &mesh, Axis axis,
                     const std::vector<float> &fractions) {
    ASSERT(! fractions.empty());

    const int dim = Util::EnumInt(axis);

    // Determine the extent of the mesh in the axis direction.
    const auto range = FindMeshExtents_(mesh, dim);

    // Convert the fractions into Y values.
    std::vector<float> slice_vals = Util::ConvertVector<float, float>(
        fractions, [&](const float &f){
            return Lerp(f, range.GetMinPoint(), range.GetMaxPoint());
        });

    // Sort the distances and add the min/max at either end. This makes the
    // bucket search have fewer special cases.
    std::sort(slice_vals.begin(), slice_vals.end());
    slice_vals.insert(slice_vals.begin(), range.GetMinPoint());
    slice_vals.push_back(range.GetMaxPoint());

    // Sort the points of the mesh into buckets based on the distances. For
    // each point, store the bucket index.
    const std::vector<float> buckets = Util::ConvertVector<float, Point3f>(
        mesh.points,
        [&](const Point3f &p){ return FindBucket_(p[dim], slice_vals); });

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

        // Copy the triangle if it does not need to be split.
        if (num_buckets <= 1) {
            new_mesh.indices.push_back(i0);
            new_mesh.indices.push_back(i1);
            new_mesh.indices.push_back(i2);
        }

        // Split if it crosses 2 or more buckets.
        else {
            // If the triangle crosses only 1 slice plane, this is easier.
            // XXXX Merge this into next case?
            if (num_buckets == 2) {
                const float slice_val = slice_vals[min + 1];
                // One of the vertices is in a different bucket.
                const int b0i = static_cast<int>(b0);
                const int b1i = static_cast<int>(b1);
                const int b2i = static_cast<int>(b2);
                if (b0i == b1i) {
                    std::cerr << "XXXX   2/" << b2i << " is different bucket\n";
                    SplitTri_(mesh, i2, i0, i1, dim, slice_val, new_mesh);
                }
                else if (b0i == b2i) {
                    std::cerr << "XXXX   1/" << b1i << " is different bucket\n";
                    SplitTri_(mesh, i1, i2, i0, dim, slice_val, new_mesh);
                }
                else {
                    ASSERT(b1i == b2i);
                    std::cerr << "XXXX   0/" << b0i << " is different bucket\n";
                    SplitTri_(mesh, i0, i1, i2, dim, slice_val, new_mesh);
                }
            }
            else {
                // Gather the Y bucket-crossing values.
                std::vector<float> tri_slice_vals;
                for (int i = min + 1; i < max; ++i)
                    tri_slice_vals.push_back(slice_vals[i]);
                // The lowest point comes first.
                if (b0 == min) {
                    SplitTri_(mesh, i0, i1, i2, dim, tri_slice_vals, new_mesh);
                }
                else if (b1 == min) {
                    SplitTri_(mesh, i1, i2, i0, dim, tri_slice_vals, new_mesh);
                }
                else {
                    ASSERT(b2 == min);
                    SplitTri_(mesh, i2, i0, i1, dim, tri_slice_vals, new_mesh);
                }
            }
        }
    }

    // Remove redundant vertices and any other issues.
    CleanMesh(new_mesh);

    SlicedMesh sliced_mesh;
    sliced_mesh.mesh  = new_mesh;
    sliced_mesh.dir   = GetAxis(dim);
    sliced_mesh.range = range;
    return sliced_mesh;
}
