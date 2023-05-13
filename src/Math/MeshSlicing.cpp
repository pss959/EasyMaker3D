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
// Slicer_ class.
// ----------------------------------------------------------------------------

class Slicer_ {
  public:
    /// Shorthand.
    typedef std::vector<float> FloatVec;

    /// The constructor is passed the input TriMesh to slice and the Axis along
    /// which to slice.
    Slicer_(const TriMesh &mesh, Axis axis);

    /// Slices the mesh using a vector of fractions with the relative distances
    /// in the range (0,1) of the planes relative to the min/max distances of
    /// the mesh along the axis. Returns the sliced mesh.
    SlicedMesh Slice(const FloatVec &fractions);

  private:
    /// Collects all info about a triangle being processed.
    struct Tri_ {
        int     dim;          ///< Copied from Slicer_.
        GIndex  i[3];         ///< Indices of result_mesh_ points.
        Point3f p[3];         ///< Points.
        float   b[3];         ///< Buckets for the three values.
        float   min, max;     ///< Min/max bucket values.
        int     num_buckets;  ///< Number of buckets crossed.

        void Print(std::ostream &out) const;
    };

    const TriMesh &input_mesh_;
    int           dim_;
    Range1f       range_;
    TriMesh       result_mesh_;

    /// Finds the min and max coordinates in the given dimension among all points
    /// in the mesh, returning the result as a Range1f.
    Range1f FindMeshExtents_() const;

    /// Returns a sorted vector of slice values in the axis dimesion, including
    /// the min and max at the ends.
    FloatVec GetSliceVals_(const FloatVec &fractions) const;

    /// Finds the bucket in which the given value lies relative to \p
    /// slice_vals (which must be sorted increasing). If the value is the same
    /// as one of the values, this returns its index. Otherwise, it returns the
    /// number halfway between the indices of the two values surrounding it.
    float FindBucket_(const FloatVec &slice_vals, float val) const;

    /// XXXX
    Tri_ BuildTri_(size_t tri_index, const FloatVec &buckets) const;

    /// Splits a triangle across the given slice values, adding the resulting
    /// points and triangles to the result mesh.
    void SplitTri_(const Tri_ &tri, int min_pt, const FloatVec &slice_vals);

    /// Splits the edge between \p p0 and \p1 based on slicing \p value. Adds
    /// the new point to the result mesh and returns its index.
    GIndex SplitEdge_(const Point3f &p0, const Point3f &p1, float value) {
        ASSERT(p0[dim_] != p1[dim_]);
        std::cerr << "XXXX SplitEdge_  " << p0 << " / " << p1
                  << " @ " << value << "\n";
        const float frac = (p0[dim_] - value) / (p0[dim_] - p1[dim_]);
        result_mesh_.points.push_back(Lerp(frac, p0, p1));
        if (result_mesh_.points.back()[1] < -30) { // XXXX
            std::cerr << "XXXX ===== BAD PT @ "
                      << result_mesh_.points.back() << "\n";
            ASSERTM(false, " ============== BAD POINT");
        }
        return result_mesh_.points.size() - 1;
    }

    /// Adds a trapezoid to the result mesh.
    void AddTrapezoid_(GIndex i0, GIndex i1, GIndex i2, GIndex i3) {
        AddTri_(i0, i1, i2);
        AddTri_(i2, i1, i3);
    }

    /// Adds a triangle to the result mesh.
    void AddTri_(GIndex i0, GIndex i1, GIndex i2) {
        result_mesh_.indices.push_back(i0);
        result_mesh_.indices.push_back(i1);
        result_mesh_.indices.push_back(i2);
    }

    /// Adds a triangle to the result mesh.
    void AddTri_(const Tri_ &tri) {
        for (int i = 0; i < 3; ++i)
            result_mesh_.indices.push_back(tri.i[i]);
    }

    /// For logging/debugging.
    friend std::ostream & operator<<(std::ostream &out, const Tri_ &tri);
};

Slicer_::Slicer_(const TriMesh &mesh, Axis axis) :
    input_mesh_(mesh),
    dim_(Util::EnumInt(axis)) {
    ASSERT(dim_ >= 0 && dim_ <= 2);

    // Determine the extent of the mesh in the axis direction.
    range_ = FindMeshExtents_();
}

SlicedMesh Slicer_::Slice(const FloatVec &fractions) {
    ASSERT(! fractions.empty());

    // Convert the fractions into slice coordinates.
    const FloatVec slice_vals = GetSliceVals_(fractions);

    std::cerr << "XXXX Slicing at values:\n";
    for (size_t i = 0; i < slice_vals.size(); ++i)
        std::cerr << "   [" << i << "] " << slice_vals[i] << "\n";

    // Sort the points of the mesh into buckets based on the distances. For
    // each point, store the bucket index.
    const FloatVec buckets = Util::ConvertVector<float, Point3f>(
        input_mesh_.points,
        [&](const Point3f &p){ return FindBucket_(slice_vals, p[dim_]); });

    result_mesh_.points.reserve(2 * input_mesh_.points.size());
    result_mesh_.indices.reserve(2 * input_mesh_.indices.size());
    result_mesh_.points = input_mesh_.points;

    // For each mesh triangle, determine if it crosses two or more buckets.
    const size_t tri_count = input_mesh_.GetTriangleCount();
    for (size_t i = 0; i < tri_count; ++i) {
        Tri_ tri = BuildTri_(i, buckets);

        std::cerr << "XXXX BUILT TRI: " << tri << "\n";

        // Add the triangle as is if it does cross at least 2 buckets.
        // Otherwise, it needs to be sliced.
        if (tri.num_buckets <= 1) {
            AddTri_(tri);
        }
        else {
            // If all 3 points are in different buckets, the triangle needs to
            // be split by the middle point.
            if (tri.b[0] != tri.b[1] &&
                tri.b[0] != tri.b[2] &&
                tri.b[1] != tri.b[2]) {
                std::cerr << "XXXX ================== NEED TO SPLIT!\n";
            }
            else {
                // Gather the bucket-crossing values.
                std::vector<float> tri_slice_vals;
                for (int i = tri.min + 1; i < tri.max; ++i)
                    tri_slice_vals.push_back(slice_vals[i]);
                // The lowest point comes first.
                ASSERT(tri.b[0] == tri.min ||
                       tri.b[1] == tri.min ||
                       tri.b[2] == tri.min);
                const int min_pt = tri.b[0] == tri.min ? 0 :
                    tri.b[1] == tri.min ? 1 : 2;
                SplitTri_(tri, min_pt, tri_slice_vals);
            }
        }
    }

    // Remove redundant vertices and any other issues.
    CleanMesh(result_mesh_);

    SlicedMesh sliced_mesh;
    sliced_mesh.mesh  = result_mesh_;
    sliced_mesh.dir   = GetAxis(dim_);
    sliced_mesh.range = range_;

    return sliced_mesh;
}

Range1f Slicer_::FindMeshExtents_() const {
    float min =  std::numeric_limits<float>::max();
    float max = -std::numeric_limits<float>::max();
    for (const auto &p: input_mesh_.points) {
        min = std::min(min, p[dim_]);
        max = std::max(max, p[dim_]);
    }
    ASSERT(min < max);
    return Range1f(min, max);
}

std::vector<float> Slicer_::GetSliceVals_(
    const std::vector<float> &fractions) const {
    // Convert the fractions into axis coordinates.
    std::vector<float> vals = Util::ConvertVector<float, float>(
        fractions, [&](const float &f){
            return Lerp(f, range_.GetMinPoint(), range_.GetMaxPoint());
        });

    // Sort the coordinates and add the min/max at either end. This makes the
    // bucket search have fewer special cases.
    std::sort(vals.begin(), vals.end());
    vals.insert(vals.begin(), range_.GetMinPoint());
    vals.push_back(range_.GetMaxPoint());

    return vals;
}

float Slicer_::FindBucket_(const FloatVec &slice_vals, float val) const {
    // Do a binary search.
    auto lower = std::lower_bound(slice_vals.begin(), slice_vals.end(), val);
    ASSERT(lower != slice_vals.end());
    const size_t index = std::distance(slice_vals.begin(), lower);
#if XXXX
    std::cerr << "XXXX Bucket for " << val << " = "
              << (val == slice_vals[index] ? index : index - .5f) << "\n";
#endif
    return val == slice_vals[index] ? index : index - .5f;
}

Slicer_::Tri_ Slicer_::BuildTri_(size_t tri_index,
                                 const FloatVec &buckets) const {
    Tri_ tri;
    tri.dim = dim_;

    for (int i = 0; i < 3; ++i) {
        const GIndex index = input_mesh_.indices[3 * tri_index + i];
        tri.i[i] = index;
        tri.p[i] = input_mesh_.points[index];
        tri.b[i] = buckets[index];
    }

    // If the minimum is in bucket N or N+.5, then the maximum has to be in
    // bucket >= N+1.5 for there to be a bucket crossing. In general, there
    // are C crossings if the maximum is at N+C+.5 or N+C+1 (which is
    // equivalent to the ceiling of the maximum at N+C+1).
    tri.min = *std::min_element(&tri.b[0], &tri.b[2]);
    tri.max = *std::max_element(&tri.b[0], &tri.b[2]);
    const auto diff = std::ceil(tri.max) - std::floor(tri.min);
    tri.num_buckets = static_cast<int>(diff);

    return tri;
}

void Slicer_::SplitTri_(const Tri_ &tri, int min_pt,
                        const FloatVec &slice_vals) {
    std::cerr << "XXXX Splitting by " << Util::JoinItems(slice_vals) << "\n";

    // All 3 triangle points, with the smallest first.
    const int t0 = min_pt;
    const int t1 = (min_pt + 1) % 3;
    const int t2 = (min_pt + 2) % 3;

    ASSERT(tri.p[t0][dim_] < slice_vals.front());
    ASSERT(tri.p[t1][dim_] > slice_vals.back() ||
           tri.p[t2][dim_] > slice_vals.back());

    // The min_pt is less than the first slicing value. If either of the other
    // two points also is less, then create a trapezoid.
    if (tri.p[t1][dim_] < slice_vals[0]) {
        // Only t2 is on the other side.
        GIndex prev0 = tri.i[t0];
        GIndex prev1 = tri.i[t1];
        for (size_t i = 0; i < slice_vals.size(); ++i) {
            // Split the edges and add the new points.
            const GIndex ni0 = SplitEdge_(tri.p[t0], tri.p[t2], slice_vals[i]);
            const GIndex ni1 = SplitEdge_(tri.p[t1], tri.p[t2], slice_vals[i]);
            AddTrapezoid_(prev0, prev1, ni0, ni1);
            prev0 = ni0;
            prev1 = ni1;
        }
        // Triangle at the max end.
        AddTri_(prev0, prev1, tri.i[t2]);
    }
    else if (tri.p[t2][dim_] < slice_vals[0]) {
        // Only t1 is on the other side.
        GIndex prev2 = tri.i[t2];
        GIndex prev0 = tri.i[t0];
        for (size_t i = 0; i < slice_vals.size(); ++i) {
            // Split the edges and add the new points.
            const GIndex ni2 = SplitEdge_(tri.p[t2], tri.p[t1], slice_vals[i]);
            const GIndex ni0 = SplitEdge_(tri.p[t0], tri.p[t1], slice_vals[i]);
            AddTrapezoid_(prev2, prev0, ni2, ni0);
            prev2 = ni2;
            prev0 = ni0;
        }
        // Triangle at the max end.
        AddTri_(prev2, prev0, tri.i[t1]);
    }
    else {  // p1 and p2 are on the other side.
        GIndex prev1 = tri.i[t1];
        GIndex prev2 = tri.i[t2];
        for (size_t i = 0; i < slice_vals.size(); ++i) {
            const GIndex ni1 = SplitEdge_(tri.p[t0], tri.p[t1], slice_vals[i]);
            const GIndex ni2 = SplitEdge_(tri.p[t0], tri.p[t2], slice_vals[i]);
            // Start with a clipped triangle; the rest are trapezoids.
            if (i == 0)
                AddTri_(tri.i[t0], ni1, ni2);
            else
                AddTrapezoid_(prev1, ni1, prev2, ni2);
            prev1 = ni1;
            prev2 = ni2;
        }
        // Trapezoid at the max end.
        AddTrapezoid_(prev1, tri.i[t1], prev2, tri.i[t2]);
    }
}

std::ostream & operator<<(std::ostream &out, const Slicer_::Tri_ &tri) {
    tri.Print(out);
    return out;
}

void Slicer_::Tri_::Print(std::ostream &out) const {
    out << "TRI[I[" << i[0] << "," << i[1] << "," << i[2] << "]"
        << " V<" << p[0][dim] << "," << p[1][dim] << "," << p[2][dim] << ">"
        << " B(" << b[0] << "," << b[1] << "," << b[2] << ")"
        << " MN=" << min
        << " MX=" << max
        << " NB=" << num_buckets
        << "]";
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

SlicedMesh SliceMesh(const TriMesh &mesh, Axis axis,
                     const std::vector<float> &fractions) {
    return Slicer_(mesh, axis).Slice(fractions);
}
