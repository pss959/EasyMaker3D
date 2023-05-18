#include "Math/MeshSlicing.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include <ion/math/vectorutils.h>

#include "Math/Linear.h"
#include "Math/MeshUtils.h"
#include "Util/Assert.h"
#include "Util/Enum.h"
#include "Util/General.h"
#include "Util/KLog.h"
#include "Util/String.h"

#include "Debug/Dump3dv.h" // XXXX

namespace {

// ----------------------------------------------------------------------------
// Slicer_ class.
// ----------------------------------------------------------------------------

class Slicer_ {
  public:
    /// Shorthand.
    typedef std::vector<float> FloatVec;

    /// Slices the given TriMesh along the given Axis. The \p fractions vector
    /// contains the relative distances in the range (0,1) of the planes
    /// relative to the min/max distances of the mesh along the axis.
    /// This returns the SlicedMesh.
    SlicedMesh Slice(const TriMesh &mesh, Axis axis, const FloatVec &fractions);

  private:
    /// Shorthand.
    typedef std::vector<GIndex> IndexVec_;

    // XXXX
    struct Tri_ {
        GIndex    a, b, c;       // Low to high; points A, B, C.
        bool      is_ccw = true; // True if ABC is counterclockwise.
        IndexVec_ ab, ac, bc;
    };

    // XXXX
    int        dim_;
    SlicedMesh sliced_mesh_;
    FloatVec   crossings_;
    FloatVec   buckets_;

    /// Finds the min and max coordinates in the given dimension among all
    /// points in the given mesh, returning the result as a Range1f.
    Range1f FindMeshExtents_(const TriMesh &mesh, int dim) const;

    /// Returns a sorted vector of coordinates where the slicing planes cross
    /// the axis dimension, including the min and max at the ends.
    FloatVec GetCrossings_(const FloatVec &fractions,
                           const Range1f &range) const;

    /// Finds the bucket in which the given value lies relative to #crossings
    /// (which is sorted increasing). If the value is the same as one of the
    /// values, this returns its index. Otherwise, it returns the number
    /// halfway between the indices of the two values surrounding it.
    float FindBucket_(float val) const;

    /// XXXX
    void ProcessTri_(const TriMesh &mesh, size_t tri_index);

    /// Fills in and returns a Tri_ instance for the indexed triangle of the
    /// given TriMesh.
    Tri_ GetTri_(const TriMesh &mesh, size_t tri_index);

    /// XXXX iextra inserts at ...
    IndexVec_ GetEdge_(const TriMesh &mesh,
                       GIndex i0, GIndex i1, int iextra = -1);

    /// Returns the indexed point from the result TriMesh. This is used in
    /// place of indexing into the original TriMesh because it allows access to
    /// newly-added points.
    const Point3f & GetPoint_(GIndex index) const {
        return sliced_mesh_.mesh.points.at(index);
    }

    /// Interpolates between \p p0 and \p p1 to find the point with the given
    /// value in the axis dimension. Adds the point to the result mesh and
    /// returns the index.
    GIndex InterpolatePoints_(const Point3f &p0, const Point3f &p1, float val) {
        const float diff = p0[dim_] - p1[dim_];
        ASSERT(std::abs(diff) > 1.e-7f);
        const float frac = (p0[dim_] - val) / diff;
        return AddPoint_(Lerp(frac, p0, p1));
    }

    /// Adds a point to the result mesh, returning its index.
    GIndex AddPoint_(const Point3f &p) {
        auto &pts = sliced_mesh_.mesh.points;
        KLOG('X', "Adding point " << pts.size() << " @ " << p);
        pts.push_back(p);
        return pts.size() - 1;
    }

    /// XXXX
    void AddQuad_(bool is_ccw, GIndex i0, GIndex i1, GIndex i2, GIndex i3) {
        AddTri_(is_ccw, i0, i1, i2);
        AddTri_(is_ccw, i0, i2, i3);
    }

    /// XXXX. Points in CCW order.
    void AddQuad_(GIndex i0, GIndex i1, GIndex i2, GIndex i3) {
        KLOG('X', "Adding quad " << i0 << " " << i1 << " " << i2 << " " << i3);
        AddTri_(i0, i1, i2);
        AddTri_(i0, i2, i3);
    }

    void AddTri_(bool is_ccw, GIndex i0, GIndex i1, GIndex i2) {
        if (is_ccw)
            AddTri_(i0, i1, i2);
        else
            AddTri_(i0, i2, i1);
    }

    /// XXXX
    void AddTri_(GIndex i0, GIndex i1, GIndex i2) {
        KLOG('X', "Adding triangle F" << sliced_mesh_.mesh.GetTriangleCount()
             << ": " << i0 << " " << i1 << " " << i2);
        sliced_mesh_.mesh.indices.push_back(i0);
        sliced_mesh_.mesh.indices.push_back(i1);
        sliced_mesh_.mesh.indices.push_back(i2);
    }
};

SlicedMesh Slicer_::Slice(const TriMesh &mesh, Axis axis,
                           const FloatVec &fractions) {
    dim_  = Util::EnumInt(axis);
    ASSERT(dim_ >= 0 && dim_ <= 2);

    sliced_mesh_.axis  = axis;

    // Determine the extent of the mesh in the axis direction.
    sliced_mesh_.range = FindMeshExtents_(mesh, dim_);

    if (fractions.empty()) {
        sliced_mesh_.mesh = mesh;
    }
    else {
        // Convert the fractions into crossing coordinates.
        crossings_ = GetCrossings_(fractions, sliced_mesh_.range);

        KLOG('X', "--- Slicing mesh in " << static_cast<char>('X' + dim_)
             << " at: " << Util::JoinItems(crossings_));

        // Find the bucket value for each mesh point.
        buckets_.reserve(mesh.points.size());
        for (size_t i = 0; i < mesh.points.size(); ++i) {
            buckets_.push_back(FindBucket_(mesh.points[i][dim_]));
            KLOG('X', "Bucket for V" << i << " = " << buckets_.back());
        }

        // Add all input mesh points to the result mesh.
        sliced_mesh_.mesh.points.reserve(2 * mesh.points.size());
        sliced_mesh_.mesh.points = mesh.points;
        sliced_mesh_.mesh.indices.reserve(2 * mesh.indices.size());

        // Process each triangle of the TriMesh.
        const size_t tri_count = mesh.GetTriangleCount();
        for (size_t i = 0; i < tri_count; ++i)
            ProcessTri_(mesh, i);

        // Clean the result to remove duplicate vertices and edges.
        CleanMesh(sliced_mesh_.mesh);
    }

    return sliced_mesh_;
}

Range1f Slicer_::FindMeshExtents_(const TriMesh &mesh, int dim) const {
    float min =  std::numeric_limits<float>::max();
    float max = -std::numeric_limits<float>::max();
    for (const auto &p: mesh.points) {
        min = std::min(min, p[dim]);
        max = std::max(max, p[dim]);
    }
    ASSERT(min < max);
    return Range1f(min, max);
}

std::vector<float> Slicer_::GetCrossings_(const std::vector<float> &fractions,
                                           const Range1f &range) const {
    // Convert the fractions into axis coordinate crossings.
    std::vector<float> vals = Util::ConvertVector<float, float>(
        fractions, [&](const float &f){
            return Lerp(f, range.GetMinPoint(), range.GetMaxPoint());
        });

    // Sort the coordinates and add the min/max at either end. This makes the
    // bucket search have fewer special cases.
    std::sort(vals.begin(), vals.end());
    vals.insert(vals.begin(), range.GetMinPoint());
    vals.push_back(range.GetMaxPoint());

    return vals;
}

float Slicer_::FindBucket_(float val) const {
    // Do a binary search.
    auto lower = std::lower_bound(crossings_.begin(), crossings_.end(), val);
    ASSERT(lower != crossings_.end());
    const size_t index = std::distance(crossings_.begin(), lower);
    return val == crossings_[index] ? index : index - .5f;
}

void Slicer_::ProcessTri_(const TriMesh &mesh, size_t tri_index) {
    // Fill in a Tri_ instance.
    const Tri_ tri = GetTri_(mesh, tri_index);

    KLOG('X', "=== TRI A=" << tri.a << " (" << GetPoint_(tri.a)[dim_]
         << ") B=" << tri.b << " (" << GetPoint_(tri.b)[dim_]
         << ") C=" << tri.c << " (" << GetPoint_(tri.c)[dim_]
         << ") CCW=" << tri.is_ccw
         << " AB=[" << Util::JoinItems(tri.ab)
         << "] AC=[" << Util::JoinItems(tri.ac)
         << "] BC=[" << Util::JoinItems(tri.bc) << "]");

    ASSERT(tri.ab.size() >= 2U);
    ASSERT(tri.ac.size() >= 2U);
    ASSERT(tri.bc.size() >= 2U);

    // If the edges all have only 2 points, add the triangle as is.
    if (tri.ab.size() == 2U && tri.ac.size() == 2U && tri.bc.size() == 2U) {
        AddTri_(tri.is_ccw, tri.a, tri.b, tri.c);
    }
    else {
        // Otherwise, scan-convert the triangle.
        //
        // Either A forms the minimum point of the triangle or C forms the
        // maximum point, or both, with B somewhere between. So there are 3
        // possibilities:
        //  1) A is the minimum with B and C at the same value.
        //  2) C is the maximum with A and B at the same value
        //  3) A is the minimum, C is the maximum, B is between.
        //
        // For case 1, side0 is AB    and side1 is AC.
        // For case 2, side0 is BC    and side1 is AC.
        // For case 3, side0 is AB+BC and side1 is AC.
        const Point3f &a = GetPoint_(tri.a);
        const Point3f &b = GetPoint_(tri.b);
        const Point3f &c = GetPoint_(tri.c);
        IndexVec_ side0;
        IndexVec_ side1;
        if (b[dim_] == c[dim_]) {
            // Case 1.
            side0 = tri.ab;
            side1 = tri.ac;
            KLOG('X', "   Case 1: side0=[" << Util::JoinItems(side0)
                 << "] side1=[" << Util::JoinItems(side1) << "]");
        }
        else if (a[dim_] == b[dim_]) {
            // Case 2.
            side0 = tri.bc;
            side1 = tri.ac;
            KLOG('X', "   Case 2: side0=[" << Util::JoinItems(side0)
                 << "] side1=[" << Util::JoinItems(side1) << "]");
        }
        else {
            // Case 3.
            ASSERT(tri.ab.back() == tri.bc.front());
            side0 = tri.ab;
            side0.pop_back();
            Util::AppendVector(tri.bc, side0);
            side1 = tri.ac;
            KLOG('X', "   Case 3: side0=[" << Util::JoinItems(side0)
                 << "] side1=[" << Util::JoinItems(side1) << "]");
        }

#if XXXX
        if (side0.size() != side1.size()) {// XXXX
            Debug::Dump3dv d("/tmp/sliced.3dv", "XXXX From MeshSlicing");
            d.SetLabelFontSize(40);
            d.SetCoincidentLabelOffset(.25f * Vector3f(1, 1, 1));
            d.AddTriMesh(sliced_mesh_.mesh);
        }
#endif

        ASSERT(side0.size() == side1.size());

        // Traverse.
        for (size_t i = 1; i < side0.size(); ++i) {
            const auto i0a = side0[i - 1];
            const auto i0b = side0[i];
            const auto i1a = side1[i - 1];
            const auto i1b = side1[i];
            if (i0a == i1a)
                AddTri_(tri.is_ccw, i0a, i0b, i1b);
            else if (i0b == i1b)
                AddTri_(tri.is_ccw, i0a, i0b, i1a);
            else
                AddQuad_(tri.is_ccw, i0a, i0b, i1b, i1a);
        }
    }
}

Slicer_::Tri_ Slicer_::GetTri_(const TriMesh &mesh, size_t tri_index) {
    Tri_ tri;

    GIndex index[3];
    for (int i = 0; i < 3; ++i)
        index[i] = mesh.indices[3 * tri_index + i];

    // Save these to detect direction.
    const GIndex i0 = index[0];
    const GIndex i1 = index[1];

    // Sort the indices increasing.
    const auto compare = [&](int a, int b){
        return mesh.points.at(a)[dim_] <= mesh.points.at(b)[dim_];
    };
    std::sort(std::begin(index), std::end(index), compare);

    // Detect counterclockwise or clockwise direction.
    for (int i = 0; i < 3; ++i) {
        if (index[i] == i0) {
            tri.is_ccw = index[(i + 1) % 3] == i1;
            break;
        }
    }

    // Store A, B, C indices.
    tri.a = index[0];
    tri.b = index[1];
    tri.c = index[2];

    // Collect indices of all relevant points along the three edges.  Insert
    // B's value into AC if it belongs there.
    tri.ab = GetEdge_(mesh, tri.a, tri.b);
    tri.ac = GetEdge_(mesh, tri.a, tri.c, tri.b);
    tri.bc = GetEdge_(mesh, tri.b, tri.c);

    return tri;
}

Slicer_::IndexVec_ Slicer_::GetEdge_(const TriMesh &mesh,
                                     GIndex i0, GIndex i1, int iextra) {
    // Use the original mesh points here so that the references remain valid
    // after inserting new points.
    const Point3f &p0 = mesh.points.at(i0);
    const Point3f &p1 = mesh.points.at(i1);

    IndexVec_ edge_indices;
    edge_indices.push_back(i0);

    // Add intermediate points at crossings, if any.
    const float d0 = p0[dim_];
    const float d1 = p1[dim_];
    if (d0 < d1) {
        const int bucket0 = static_cast<int>(std::floor(buckets_.at(i0)));
        const int bucket1 = static_cast<int>(std::ceil(buckets_.at(i1)));
        for (int i = bucket0; i <= bucket1; ++i) {
            const float crossing = crossings_.at(i);
            if (crossing > d0 && crossing < d1)
                edge_indices.push_back(InterpolatePoints_(p0, p1, crossing));
        }
    }

    edge_indices.push_back(i1);

    // If there is an extra value to insert, see if it is already there.
    if (iextra >= 0) {
        const float pval = GetPoint_(iextra)[dim_];
        int found_index = -1;
        if (pval > GetPoint_(edge_indices.front())[dim_] &&
            pval < GetPoint_(edge_indices.back())[dim_]) {
            for (size_t i = 1; i < edge_indices.size(); ++i) {
                const float val = GetPoint_(edge_indices[i])[dim_];
                // Already there? No need to insert.
                if (pval == val) {
                    break;
                }
                else if (pval > val) {
                    found_index = i - 1;
                    break;
                }
            }
            if (found_index >= 0) {
                // Add a point at the same value.
                const GIndex index = InterpolatePoints_(p0, p1, pval);
                edge_indices.insert(edge_indices.begin() + found_index, index);
                KLOG('X', "-- Inserted " << index << " to get ["
                     << Util::JoinItems(edge_indices) << "]");
            }
        }
    }

    return edge_indices;
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

SlicedMesh SliceMesh(const TriMesh &mesh, Axis axis,
                     const std::vector<float> &fractions) {
    return Slicer_().Slice(mesh, axis, fractions);
}

SlicedMesh SliceMesh(const TriMesh &mesh, Axis axis, size_t num_slices) {
    ASSERT(num_slices >= 1U);
    // Construct a vector of fractions.
    std::vector<float> fractions(num_slices - 1, 0.f);
    for (size_t i = 1; i < num_slices; ++i)
        fractions[i - 1] = static_cast<float>(i) / num_slices;
    return SliceMesh(mesh, axis, fractions);
}
