#include "Math/Beveler.h"

#include <cmath>
#include <unordered_map>
#include <unordered_set>

#include <ion/math/vectorutils.h>

#include "Math/Linear.h"
#include "Math/PolyMesh.h"
#include "Math/PolyMeshBuilder.h"
#include "Math/PolyMeshMerging.h"
#include "Math/VertexRing.h"
#include "Util/Assert.h"

namespace {

// ----------------------------------------------------------------------------
// Helper classes and types.
// ----------------------------------------------------------------------------

using ion::math::AngleBetween;
using ion::math::Cross;
using ion::math::Dot;
using ion::math::Normalized;

typedef PolyMesh::Edge      Edge;       ///< Convenience typedef.
typedef PolyMesh::EdgeVec   EdgeVec;    ///< Convenience typedef.
typedef PolyMesh::Face      Face;       ///< Convenience typedef.
typedef PolyMesh::FaceVec   FaceVec;    ///< Convenience typedef.
typedef PolyMesh::Vertex    Vertex;     ///< Convenience typedef.
typedef PolyMesh::VertexVec VertexVec;  ///< Convenience typedef.
typedef PolyMesh::IndexVec  IndexVec;   ///< Convenience typedef.

/// Orientations for a profile at an edge. Orientation_::kLeftToRight means the
/// profile is applied from the left border to the right.
enum class Orientation_ {
    kUnknown,
    kLeftToRight,
    kRightToLeft
};

/// Returns the opposite of an orientation, assuming it is known.
static Orientation_ GetOppositeOrientation_(Orientation_ o) {
    return o == Orientation_::kUnknown ? o :
        o == Orientation_::kLeftToRight ? Orientation_::kRightToLeft :
        Orientation_::kLeftToRight;
}

/// A bevel Profile always starts at (0,0) and ends at scale*(1,1). These
/// endpoints will lie on two border lines parallel to an edge that lie in the
/// faces adjacent to the edge. (If there are only 2 profile points, these
/// border lines form the appropriate edge bevel.)
///
/// The EdgeFrame_ struct stores the vectors separating the two border lines
/// from the line containing the edge. When traveling from v0 to v1 along the
/// edge, the left border is to the left and the right border is to the right.
/// Therefore, the left border is in the plane of the edge's face and the right
/// border is in the plane of the opposite edge's face.
struct EdgeFrame_ {
    /// Unit vector from the edge toward its left border.
    Vector3f l_border_vec;

    /// Unit vector from the edge toward its right border.
    Vector3f r_border_vec;

    /// Indicates how to orient profiles to create better symmetry.
    Orientation_ orientation = Orientation_::kUnknown;

    /// Closest point of the edge's left border to the left border of the
    /// previous edge in its face.
    Point3f closest_l_point;

    /// Closest point of the edge's right border to the right border of the
    /// previous edge in its face.
    Point3f closest_r_point;

    /// True if closest_l_point has been set.
    bool closest_l_point_set = false;

    /// True if closest_r_point has been set.
    bool closest_r_point_set = false;

    /// Creates and returns an EdgeFrame_ to use for the opposite edge.
    EdgeFrame_ * GetOpposite() const {
        EdgeFrame_ *opp = new EdgeFrame_;
        opp->l_border_vec = r_border_vec;
        opp->r_border_vec = l_border_vec;
        opp->orientation  = GetOppositeOrientation_(orientation);
        return opp;
    }

    /// Returns true if the edge has a bevel applied to it.
    bool IsBeveled() const { return l_border_vec != r_border_vec; }
};

// ----------------------------------------------------------------------------
// Beveler_ class.
// ----------------------------------------------------------------------------

/// This class does most of the work. It uses the PolyMesh data structure to
/// optimize the work and the VertexRing class to handle much of the internal
/// details.
///
/// The general strategy is to offset each edge to be beveled in both
/// directions to create border lines, then intersect the border lines to get
/// bevel end points. The internal EdgeFrame_ class stores the closest
/// intersection points for an edge's left and right border lines as
/// closest_left_point and closest_right_point. These points are used to
/// construct VertexRing instances for each vertex of the PolyMesh. The
/// vertices in the VertexRing instances are then connected into faces: there
/// is one face resulting from each original face of the PolyMesh, M faces
/// resulting from each original beveled edge, and N faces from each
/// vertex. The values of M and N depend on the number of points in the Bevel's
/// Profile.
class Beveler_ {
  public:
    /// The constructor is passed the Bevel and PolyMeshBuilder used to build
    /// the beveled PolyMesh.
    Beveler_(const Bevel &bevel) : bevel_(bevel) {}

    /// Builds and returns a beveled version of the given PolyMesh.
    PolyMesh ApplyBevel(const PolyMesh &poly_mesh);

  private:
    /// Maps an Edge to its corresponding EdgeFrame_.
    typedef std::unordered_map<const Edge *, EdgeFrame_ *> FrameMap_;

    /// Maps a Vertex to its corresponding VertexRing.
    typedef std::unordered_map<const Vertex *, VertexRing *> RingMap_;

    /// Bevel being applied.
    const Bevel &bevel_;

    /// PolyMeshBuilder used to construct the beveled PolyMesh.
    PolyMeshBuilder builder_;

    /// Constructs an EdgeFrame_ for each edge, storing them in the given
    /// FrameMap_.
    void BuildEdgeFrames_(const EdgeVec &edges, FrameMap_ &frame_map);

    /// Builds and returns an EdgeFrame_ for the given edge.
    EdgeFrame_ * BuildEdgeFrame_(const Edge &edge);

    /// Finds the intersection points for all edge borders, storing the results
    /// in the closest_l_point and closest_r_point fields of each EdgeFrame_.
    void FindEdgeBorderPoints_(const EdgeVec &edges, FrameMap_ &frame_map);

    /// Assigns orientations to all edges, storing them in the EdgeFrame_ per
    /// edge. This tries to produce a consistent and aesthetic orientation.
    void AssignOrientations_(const EdgeVec &edges, FrameMap_ &frame_map);

    /// Recursive function that assigns an orientation and then proceeds to
    /// neighboring edges.
    void AssignOrientation_(Edge &edge, Orientation_ orientation,
                            FrameMap_ &frame_map);

    /// Finds the closest point between the left border lines of edge ee and
    /// the previous edge in ee's face, storing the result as closest_l_point
    /// in ee's frame and closest_r_point in the frame for the edge opposite
    /// the previous edge.
    void FindClosestBorderPoints_(const Edge &ee, FrameMap_ &frame_map);

    /// Sets closest_l_point (when is_left is true) or closest_r_point in the
    /// FrameMap_ for the given edge and also sets the point on the opposite
    /// side for the opposite edge on the previous or next edge.
    void SetClosestPoint_(const Edge &e, const Point3f &pt, bool is_left,
                          FrameMap_ &frame_map);

    /// Builds a VertexRing representing the new points surrounding each
    /// vertex, adding each to the given RingMap_.
    void BuildVertexRings_(const EdgeVec &edges, const FrameMap_ &frame_map,
                           RingMap_ &ring_map);

    /// Builds a single VertexRing for the given edge's v0.
    VertexRing * BuildVertexRing_(Edge &edge, const FrameMap_ &frame_map);

    /// Computes and returns the base point for an edge's profile from its
    /// closest border points.
    Point3f ComputeBasePoint_(const Edge &edge,
                              const Point3f &lp, const Point3f &rp);

    /// Adds new faces corresponding to old faces using the VertexRings.
    void AddFaceFaces_(const FaceVec &faces, const RingMap_ &ring_map);

    /// Returns indices for all vertices contributed by the given Face to all
    /// VertexRings around the starting Vertex (v0) of each Edge in the
    /// EdgeVec.  This may be a face border or a hole.
    IndexVec GetVertexIndices_(const Face &face, const EdgeVec &edges,
                               const RingMap_ &ring_map);

    /// Cleans up PolyMeshBuilder vertices for a polygon specified as a list of
    /// indices. Returns a new list of indices after removing any vertex whose
    /// incoming and outgoing edges are collinear.
    IndexVec CleanVertices_(const IndexVec &indices);

    /// Adds new faces corresponding to old edges to the PolyMeshBuilder.
    void AddEdgeFaces_(const EdgeVec &edges, const RingMap_ &ring_map);
};

// ----------------------------------------------------------------------------
// Beveler_ implementation.
// ----------------------------------------------------------------------------

PolyMesh Beveler_::ApplyBevel(const PolyMesh &poly_mesh) {
    // Create an EdgeFrame_ for each edge.
    FrameMap_ frame_map;
    BuildEdgeFrames_(poly_mesh.edges, frame_map);

    // Assign orientations to each EdgeFrame_.
    AssignOrientations_(poly_mesh.edges, frame_map);

    // Compute the points for all EdgeFrame_ pairs.
    FindEdgeBorderPoints_(poly_mesh.edges, frame_map);

    // Build a VertexRing for each Vertex and store them in a RingMap_.
    RingMap_ ring_map;
    BuildVertexRings_(poly_mesh.edges, frame_map, ring_map);

    AddFaceFaces_(poly_mesh.faces, ring_map);
    AddEdgeFaces_(poly_mesh.edges, ring_map);

    for (auto &v: poly_mesh.vertices) {
        VertexRing &vr = *ring_map.at(v);
        if (vr.HasArea())
            vr.AddPolygons(bevel_);
    }

    // Delete all VertexRing and EdgeFrame_ instances.
    for (auto it: ring_map)
        delete it.second;
    for (auto it: frame_map)
        delete it.second;

    // Build and return a PolyMesh from the results.
    return builder_.BuildPolyMesh();
}

void Beveler_::BuildEdgeFrames_(const EdgeVec &edges, FrameMap_ &frame_map) {
    for (auto &edge: edges) {
        if (! Util::MapContains(frame_map, edge)) {
            EdgeFrame_ *frame = BuildEdgeFrame_(*edge);
            frame_map[edge]                = frame;
            frame_map[edge->opposite_edge] = frame->GetOpposite();
        }
    }
}

EdgeFrame_ * Beveler_::BuildEdgeFrame_(const Edge &edge) {
    EdgeFrame_ *frame = new EdgeFrame_;

    // Determine whether the edge should have the profile applied, based on the
    // angle formed at the edge. Note that the angle between the faces is the
    // supplementary angle of the angle between the normals.
    const Vector3f edge_vec = edge.GetUnitVector();
    const Vector3f l_normal  = edge.face->GetNormal();
    const Vector3f r_normal  = edge.opposite_edge->face->GetNormal();

    // Compute the angle formed by the edge's faces.
    const Anglef edge_angle =
        Anglef::FromDegrees(180) - AngleBetween(l_normal, r_normal);
    const bool is_beveled = edge_angle <= bevel_.max_angle;

    if (is_beveled) {
        // The profile is defined in the plane perpendicular to the edge,
        // extending from the edge's face to the opposite edge's face. Get the
        // two unit vectors in the plane for the faces: each is perpendicular
        // to both the edge and the face normal.
        frame->l_border_vec = Normalized(Cross(l_normal, edge_vec));
        frame->r_border_vec = Normalized(Cross(edge_vec, r_normal));
    }
    else {
        // If the edge is not profiled, the borders lie on the edge.
        frame->l_border_vec.Set(0, 0, 0);
        frame->r_border_vec.Set(0, 0, 0);
    }

    // closest_l_point and closest_r_point are set later once all border
    // vectors are known.
    return frame;
}

void Beveler_::FindEdgeBorderPoints_(const EdgeVec &edges,
                                     FrameMap_ &frame_map) {
    for (auto &edge: edges)
        FindClosestBorderPoints_(*edge, frame_map);
}

void Beveler_::AssignOrientations_(const EdgeVec &edges, FrameMap_ &frame_map) {
    for (auto &edge: edges)
        AssignOrientation_(*edge, Orientation_::kLeftToRight, frame_map);
}

void Beveler_::AssignOrientation_(Edge &edge, Orientation_ orientation,
                                  FrameMap_ &frame_map) {
    // Do nothing if already assigned.
    EdgeFrame_ &e_frame = *frame_map.at(&edge);
    if (e_frame.orientation != Orientation_::kUnknown)
        return;

    EdgeFrame_ &o_frame = *frame_map.at(edge.opposite_edge);
    Orientation_ opp_orientation = GetOppositeOrientation_(orientation);
    e_frame.orientation = orientation;
    o_frame.orientation = opp_orientation;

    // Unprofiled edges do not affect any others. For profiled edges, go to
    // neighbors and try to apply a consistent orientation.
    if (e_frame.IsBeveled()) {
        AssignOrientation_(edge.NextEdgeInFace(), orientation, frame_map);
        AssignOrientation_(edge.opposite_edge->NextEdgeInFace(),
                           opp_orientation, frame_map);
    }
}

void Beveler_::BuildVertexRings_(const EdgeVec &edges,
                                 const FrameMap_ &frame_map,
                                 RingMap_ &ring_map) {
    for (auto &edge: edges) {
        // Process each vertex only once.
        if (! Util::MapContains(ring_map, edge->v0))
            ring_map[edge->v0] = BuildVertexRing_(*edge, frame_map);
    }
}

VertexRing * Beveler_::BuildVertexRing_(Edge &edge,
                                        const FrameMap_ &frame_map) {
    VertexRing *vr = new VertexRing(*edge.v0, builder_,
                                    bevel_.profile.GetPointCount());

    // Position and add all the profile points for each edge around the vertex.
    auto edges = PolyMesh::GetVertexEdges(edge);
    std::reverse(edges.begin(), edges.end());
    for (auto &e: edges) {
        EdgeFrame_ &frame = *frame_map.at(e);
        ASSERT(frame.closest_l_point_set);
        ASSERT(frame.closest_r_point_set);
        const Point3f &lp = frame.closest_l_point;
        const Point3f &rp = frame.closest_r_point;

        // If the edge is not profiled, just add the appropriate number of
        // copies of the point to the VertexRing.
        std::vector<Point3f> pts;
        if (! frame.IsBeveled()) {
            pts.assign(bevel_.profile.GetPointCount(), lp);
        }
        else {
            // Otherwise, compute a base point on the edge.
            const Point3f base_point = ComputeBasePoint_(*e, lp, rp);
            const Vector3f lvec = (lp - base_point) / bevel_.scale;
            const Vector3f rvec = (rp - base_point) / bevel_.scale;
            ASSERT(frame.orientation != Orientation_::kUnknown);
            if (frame.orientation == Orientation_::kRightToLeft) {
                pts = VertexRing::ApplyBevelToPoint(bevel_, base_point,
                                                    rvec, lvec);
                std::reverse(pts.begin(), pts.end());
            }
            else {
                pts = VertexRing::ApplyBevelToPoint(bevel_, base_point,
                                                    lvec, rvec);
            }
        }
        const bool is_reversed =
            frame.orientation == Orientation_::kRightToLeft;
        vr->AddEdgeProfilePoints(*e, is_reversed, pts);
    }

    // Let the vertex ring know that all edges have been added.
    vr->EndEdges();

    return vr;
}

Point3f Beveler_::ComputeBasePoint_(const Edge &edge,
                                    const Point3f &lp, const Point3f &rp) {
    // The line containing lp and rp represents the profile direction.
    const Vector3f prof_dir = Normalized(rp - lp);

    // For a regular profiled edge, the profile direction will be perpendicular
    // to the edge. In this case, just use the closest point on the edge to the
    // line containing lp and rp.
    const Vector3f edge_dir = edge.GetUnitVector();
    if (std::fabs(Dot(prof_dir, edge_dir)) < .001f) { // Perpendicular.
        Point3f close0;
        Point3f close1;
        GetClosestLinePoints(edge.v0->point, edge_dir, lp, prof_dir,
                             close0, close1);
        return close0;
    }

    // In the other case, the profile will lie in the plane containing v0, rp,
    // and lp. Use v0, which is the intersection of the edge with this plane.
    else {
        return edge.v0->point;
    }
}

void Beveler_::AddFaceFaces_(const FaceVec &faces, const RingMap_ &ring_map) {
    for (auto &face: faces) {
        builder_.AddPolygon(
            GetVertexIndices_(*face, face->outer_edges, ring_map));
        for (auto &hole: face->hole_edges)
            builder_.AddHole(GetVertexIndices_(*face, hole, ring_map));
    }
}

IndexVec Beveler_::GetVertexIndices_(const Face &face, const EdgeVec &edges,
                                     const RingMap_ &ring_map) {
    IndexVec indices;
    for (auto &edge: edges) {
        VertexRing &vr = *ring_map.at(edge->v0);

        // Get indices of all ring points on this face. There should be 1 or 2.
        IndexVec ring_indices = vr.GetFacePointIndices(face);
        ASSERT(ring_indices.size() == 1U || ring_indices.size() == 2U);
        Util::AppendVector(ring_indices, indices);
    }

    // Get rid of problematic vertices and edges.
    return CleanVertices_(indices);
}

IndexVec Beveler_::CleanVertices_(const IndexVec &indices) {
    IndexVec clean;
    for (size_t i = 0; i < indices.size(); ++i) {
        // Determine if Vertex i is ok: its incoming and outgoing edges are not
        // collinear.
        const size_t i0   = i == 0 ? indices.back() : indices[i - 1];
        const size_t i1   = indices[i];
        const size_t i2   = indices[i == indices.size() - 1 ? 0 : i + 1];
        const Point3f &p0 = builder_.GetVertex(i0);
        const Point3f &p1 = builder_.GetVertex(i1);
        const Point3f &p2 = builder_.GetVertex(i2);
        if (std::fabs(Dot(Normalized(p0 - p1), Normalized(p2 - p1))) < .999f)
            clean.push_back(indices[i]);
    }
    return clean;
}

void Beveler_::AddEdgeFaces_(const EdgeVec &edges, const RingMap_ &ring_map) {
    std::unordered_set<Edge *> processed_edges;
    for (auto &edge: edges) {
        // Do this once per edge pair.
        if (Util::MapContains(processed_edges, edge))
            continue;
        processed_edges.insert(edge);
        processed_edges.insert(edge->opposite_edge);

        VertexRing &vr0 = *ring_map.at(edge->v0);
        VertexRing &vr1 = *ring_map.at(edge->v1);

        // There should be at least two ring points at each end that have the
        // edge in them. The number of points should match.
        const IndexVec i0 = vr0.GetEdgePointIndices(*edge);
        const IndexVec i1 = vr1.GetEdgePointIndices(*edge);
        ASSERT(i0.size() >= 2U);
        ASSERT(i0.size() == i1.size());

        // Connect them with faces. Since the vertex rings are oriented in
        // opposite directions, traverse the lists in opposite order.
        for (size_t i = 1; i < i0.size(); ++i) {
            const size_t j = i1.size() - i;
            builder_.AddQuad(i0[i - 1], i0[i], i1[j - 1], i1[j]);
        }
    }
}

void Beveler_::FindClosestBorderPoints_(const Edge &ee, FrameMap_ &frame_map) {
    // Do nothing if this was already done.
    EdgeFrame_ &ee_frame = *frame_map.at(&ee);
    if (ee_frame.closest_l_point_set)
        return;

    // The starting point is the closest point between the profile border lines
    // for this edge and the previous edge (which meets this edge at v0).
    const Edge &pe = ee.PreviousEdgeInFace();
    EdgeFrame_ &pe_frame = *frame_map.at(&pe);

    // Get a point on each edge's inside border line.
    const Point3f ebp = ee.v0->point + bevel_.scale * ee_frame.l_border_vec;
    const Point3f pbp = pe.v0->point + bevel_.scale * pe_frame.l_border_vec;

    // Get the direction of each edge's border line (same as edge).
    const Vector3f ev = ee.GetUnitVector();
    const Vector3f pv = pe.GetUnitVector();

    // Find the closest point on ee's border line. (The border lines should
    // intersect, so the closest points should be nearly identical.)
    Point3f close0;
    Point3f close1;
    GetClosestLinePoints(ebp, ev, pbp, pv, close0, close1);
    SetClosestPoint_(ee, close0, true, frame_map);

    // If either edge is not profiled, also set the closest point on the other
    // side, since we want them to be exactly the same.
    if (! ee_frame.IsBeveled())
        SetClosestPoint_(ee, close0, false, frame_map);
    if (! pe_frame.IsBeveled())
        SetClosestPoint_(*pe.opposite_edge, close0, true, frame_map);

}

void Beveler_::SetClosestPoint_(const Edge &e, const Point3f &pt,
                                bool is_left, FrameMap_ &frame_map) {
    EdgeFrame_ &e_frame = *frame_map.at(&e);

    if (is_left) {
        const Edge &other   = *e.PreviousEdgeInFace().opposite_edge;
        EdgeFrame_ &o_frame = *frame_map.at(&other);
        e_frame.closest_l_point     = pt;
        e_frame.closest_l_point_set = true;
        o_frame.closest_r_point     = pt;
        o_frame.closest_r_point_set = true;
    }
    else {
        const Edge &other = e.opposite_edge->NextEdgeInFace();
        EdgeFrame_ &o_frame = *frame_map.at(&other);
        e_frame.closest_r_point     = pt;
        e_frame.closest_r_point_set = true;
        o_frame.closest_l_point     = pt;
        o_frame.closest_l_point_set = true;
    }
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Beveler class functions.
// ----------------------------------------------------------------------------

TriMesh Beveler::ApplyBevel(const TriMesh &mesh, const Bevel &bevel) {
    // Convert to PolyMesh.
    PolyMesh poly_mesh(mesh);

    // Merge any coplanar faces.
    MergeCoplanarFaces(poly_mesh);

    // Apply the bevel to get a new PolyMesh.
    PolyMesh beveled = Beveler_(bevel).ApplyBevel(poly_mesh);

    // Convert back to a TriMesh.
    return beveled.ToTriMesh();
}

PolyMesh Beveler::ApplyBevel(const PolyMesh &poly_mesh, const Bevel &bevel) {
    return Beveler_(bevel).ApplyBevel(poly_mesh);
}
