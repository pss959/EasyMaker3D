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
    typedef std::vector<GIndex> IndexVec_;

    /// A Tri_ represents a triangle from the original TriMesh to be processed.
    struct Tri_ {
        /// \name Triangle vertices
        /// The three vertices of the triangle, sorted by increasing coordinate
        /// value in the slicing axis dimension.
        ///@{
        GIndex    a, b, c;       // Low to high; points A, B, C.
        ///@}

        /// True if the triangle ABC is traversed counterclockwise (the same as
        /// triangles in the original TriMesh).
        bool      is_ccw = true;

        /// TriMesh indices of vertices along the AB, AC, and BC edges. There
        /// are always at least 2 indices (the vertices of the original
        /// triangle edges). Any interior indices refer to new points created
        /// in the result TriMesh.
        IndexVec_ ab, ac, bc;
    };

    /// Slicing axis dimension.
    int        dim_;

    /// Vector of coordinate values in the slicing dimension where the slicing
    /// planes are, in increasing order. The first and last values in the
    /// vector are the min and max values of the original TriMesh; these make
    /// it much easier to find buckets.
    FloatVec   crossings_;

    /// Bucket values for each point of the original TriMesh. Bucket B is
    /// between #crossings_ B and #crossings_ B+1. If the point is exactly on
    /// the #crossings_ value B, the bucket value is B. If it is greater than
    /// #crossings_ B but less than #crossings_ B+1, the bucket value is B+.5.
    /// The bucket values are used to more efficiently find which slicing
    /// planes intersect an edge of the TriMesh.
    FloatVec   buckets_;

    /// The resulting SlicedMesh.
    SlicedMesh sliced_mesh_;

    /// Finds the min and max coordinates in the given dimension among all
    /// points in the given mesh, returning the result as a Range1f.
    Range1f FindMeshExtents_(const TriMesh &mesh, int dim) const;

    /// Does the main work of slicing the TriMesh at the given \p fractions
    /// (not empty).
    void SliceMesh_(const TriMesh &mesh, const FloatVec &fractions);

    /// Returns a sorted vector of coordinates where the slicing planes cross
    /// the axis dimension, including the min and max at the ends.
    FloatVec GetCrossings_(const FloatVec &fractions,
                           const Range1f &range) const;

    /// Finds the bucket in which the given value lies relative to #crossings_
    /// (which is sorted increasing). If the value is the same as one of the
    /// values, this returns its index. Otherwise, it returns the number
    /// halfway between the indices of the two values surrounding it.
    float FindBucket_(float val) const;

    /// Fills in and returns a Tri_ instance for the indexed triangle of the
    /// given TriMesh.
    Tri_ GetTri_(const TriMesh &mesh, size_t tri_index);

    /// Returns indices for all points on an edge of the mesh between the
    /// vertex points indexed by \p i0 and \p i1. This creates and adds new
    /// points to the result mesh if necessary where slicing planes intersect
    /// the edge.
    IndexVec_ GetEdgeIndices_(const TriMesh &mesh, GIndex i0, GIndex i1);

    /// Interpolates between mesh points indexed by \p i0 and \p i1 to find the
    /// point with the given value in the axis dimension. Adds the point to the
    /// result mesh and returns the index. This is used to slice an edge at a
    /// crossing value.
    GIndex InterpolatePoints_(GIndex i0, GIndex i1, float val);

    /// Processes the mesh triangle represented by \p tri, adding to the
    /// result TriMesh.
    void ProcessTri_(const Tri_ &tri);

    /// Traverses the points indexed by \p side0 and \p side1 to add new
    /// triangles to the result mesh.
    void TraverseTriSides_(bool is_ccw,
                           const IndexVec_ &side0, const IndexVec_&side1);

    /// Returns the indexed point from the result TriMesh. This is used in
    /// place of indexing into the original TriMesh because it allows access to
    /// newly-added points. Note that this returns a reference that could be
    /// invalidated after other points are added.
    const Point3f & GetPoint_(GIndex index) const {
        return sliced_mesh_.mesh.points.at(index);
    }

    /// Adds a point to the result mesh, returning its index.
    GIndex AddPoint_(const Point3f &p) {
        auto &pts = sliced_mesh_.mesh.points;
        pts.push_back(p);
        return pts.size() - 1;
    }

    /// Adds a quadrilateral to the result mesh. The vertex indices are given
    /// in clockwise or counterclockwise order, depending on \p is_ccw.
    void AddQuad_(bool is_ccw, GIndex i0, GIndex i1, GIndex i2, GIndex i3) {
        AddTri_(is_ccw, i0, i1, i2);
        AddTri_(is_ccw, i0, i2, i3);
    }

    /// Adds a triangle to the result mesh. The vertex indices are given in
    /// clockwise or counterclockwise order, depending on \p is_ccw.
    void AddTri_(bool is_ccw, GIndex i0, GIndex i1, GIndex i2) {
        if (is_ccw)
            AddCCWTri_(i0, i1, i2);
        else
            AddCCWTri_(i0, i2, i1);
    }

    /// Adds a triangle to the result mesh. The vertex indices are given in
    /// counterclockwise order.
    void AddCCWTri_(GIndex i0, GIndex i1, GIndex i2) {
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
        SliceMesh_(mesh, fractions);

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

void Slicer_::SliceMesh_(const TriMesh &mesh, const FloatVec &fractions) {
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

    // Fill in and process a Tri_ instance for each triangle of the original
    // mesh to add to the result mesh.
    const size_t tri_count = mesh.GetTriangleCount();
    for (size_t i = 0; i < tri_count; ++i)
        ProcessTri_(GetTri_(mesh, i));
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

Slicer_::Tri_ Slicer_::GetTri_(const TriMesh &mesh, size_t tri_index) {
    Tri_ tri;

    GIndex index[3];
    for (int i = 0; i < 3; ++i)
        index[i] = mesh.indices[3 * tri_index + i];

    // Save these to detect direction.
    const GIndex i0 = index[0];
    const GIndex i1 = index[1];

    // Sort the indices increasing.
    const auto is_less = [&](int a, int b){
        return mesh.points.at(a)[dim_] <= mesh.points.at(b)[dim_];
    };
    std::sort(std::begin(index), std::end(index), is_less);

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

    // Collect indices of all relevant points along the three edges.
    tri.ab = GetEdgeIndices_(mesh, tri.a, tri.b);
    tri.bc = GetEdgeIndices_(mesh, tri.b, tri.c);
    tri.ac = GetEdgeIndices_(mesh, tri.a, tri.c);

    return tri;
}

Slicer_::IndexVec_ Slicer_::GetEdgeIndices_(const TriMesh &mesh,
                                            GIndex i0, GIndex i1) {
    IndexVec_ edge_indices;
    edge_indices.push_back(i0);

    // Add intermediate points at crossings, if any.
    const float d0 = mesh.points.at(i0)[dim_];
    const float d1 = mesh.points.at(i1)[dim_];
    if (d0 < d1) {
        const int min_bucket = static_cast<int>(std::floor(buckets_.at(i0)));
        const int max_bucket = static_cast<int>(std::ceil(buckets_.at(i1)));
        for (int i = min_bucket; i <= max_bucket; ++i) {
            const float crossing = crossings_.at(i);
            if (crossing > d0 && crossing < d1)
                edge_indices.push_back(InterpolatePoints_(i0, i1, crossing));
        }
    }

    edge_indices.push_back(i1);
    return edge_indices;
}

GIndex Slicer_::InterpolatePoints_(GIndex i0, GIndex i1, float val) {
    const Point3f &p0 = GetPoint_(i0);
    const Point3f &p1 = GetPoint_(i1);
    const float diff = p0[dim_] - p1[dim_];
    ASSERT(std::abs(diff) > 1.e-7f);

    const float frac = (p0[dim_] - val) / diff;
    const Point3f p = Lerp(frac, p0, p1);
    const GIndex index = AddPoint_(p);

    KLOG('X', "Interp V" << index << " @ " << p << " from V"
         << i0 << " to V" << i1 << " at " << val);
    return index;
}

void Slicer_::ProcessTri_(const Tri_ &tri) {
    KLOG('X', "=== TRI ABC=[V" << tri.a << " V" << tri.b << " V" << tri.c
         << "] @(" << GetPoint_(tri.a)[dim_]
         << ", " << GetPoint_(tri.b)[dim_]
         << ", " << GetPoint_(tri.c)[dim_]
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
        // Otherwise, need to traverse the vertices forming the edges of the
        // triangle. Collect the indices for both sides of the traversal:

        // side0 is AB + BC.
        ASSERT(tri.ab.back() == tri.bc.front());
        IndexVec_ side0 = tri.ab;
        side0.pop_back();
        Util::AppendVector(tri.bc, side0);

        // side1 is AC.
        IndexVec_ side1 = tri.ac;
        KLOG('X', "   side0=[" << Util::JoinItems(side0)
             << "] side1=[" << Util::JoinItems(side1) << "]");

        // Traverse them.
        TraverseTriSides_(tri.is_ccw, side0, side1);
    }
}

void Slicer_::TraverseTriSides_(bool is_ccw,
                                const IndexVec_ &side0, const IndexVec_&side1) {
    // If one side has more vertices than the other (by exactly 1), start by
    // creating a triangle connecting the first 2 from the longer side with the
    // first one from the shorter side. Then proceed as normal.
    const size_t size0 = side0.size();
    const size_t size1 = side1.size();
    size_t i0 = 1;
    size_t i1 = 1;
    if (size0 != size1) {
        if (size0 > size1) {
            ASSERT(size0 - size1 == 1U);
            AddTri_(is_ccw, side0[0], side0[1], side1[0]);
            i0 = 2;
        }
        else {
            ASSERT(size1 - size0 == 1U);
            AddTri_(is_ccw, side0[0], side1[1], side1[0]);
            i1 = 2;
        }
    }

    // Traverse the rest.
    while (i0 < side0.size() && i1 < side1.size()) {
        const auto i0a = side0[i0 - 1];
        const auto i0b = side0[i0];
        const auto i1a = side1[i1 - 1];
        const auto i1b = side1[i1];
        if (i0a == i1a)
            AddTri_(is_ccw, i0a, i0b, i1b);
        else if (i0b == i1b)
            AddTri_(is_ccw, i0a, i0b, i1a);
        else
            AddQuad_(is_ccw, i0a, i0b, i1b, i1a);
        ++i0;
        ++i1;
    }
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
