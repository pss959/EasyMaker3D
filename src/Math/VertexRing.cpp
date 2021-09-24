#include "Math/VertexRing.h"

#include <algorithm>

#include <ion/math/vectorutils.h>

#include "Math/Linear.h"
#include "Util/General.h"

// ----------------------------------------------------------------------------
// VertexRing::Point_ class.
// ----------------------------------------------------------------------------

//! A VertexRing contains a series of points representing vertices of bevel
//! edges. There are two types of points, each with a derived class.
struct VertexRing::Point_ {
  public:
    //! 3D position.
    Point3f pos;

    //! Index of point in PolyMeshBuilder. This is set when the point is added
    //! to the PolyMeshBuilder.
    int index = -1;

    //! Tracks the number of times a point has been repositioned. This should
    //! always remain 0 for points in the outer ring. This should end up as 2
    //! for all points in an inner ring.
    int reposition_count = 0;

    Point_(const Point3f &pos_in) : pos(pos_in) {}

    virtual ~Point_() {}
};

// ----------------------------------------------------------------------------
// VertexRing::BoundaryPoint_ class.
// ----------------------------------------------------------------------------

//! A BoundaryPoint_ lies on a face of the original model and typically
//! joins two bevels. It corresponds to an end point of a bevel Profile.
//! There is a special case where two profiles do not meet at a point. In
//! this case, the end_point will not be null.
struct VertexRing::BoundaryPoint_ : public Point_ {
  public:
    const Face *face;             //!< Face the point is in.
    const Edge *end_edge;         //!< Edge ending at the boundary.
    const Edge *start_edge;       //!< Edge starting at the boundary.
    bool       is_edge_reversed;  //!< True if edge is reversed.

    //! This is used for the special case. It is usually null.
    Point_     *end_point = nullptr;

    BoundaryPoint_(const Face &face_in, const Edge *end_edge_in,
                   const Edge *start_edge_in, const Point3f &pos_in) :
        Point_(pos_in),
        face(&face_in),
        end_edge(end_edge_in),
        start_edge(start_edge_in),
        is_edge_reversed(false) {
    }
};

// ----------------------------------------------------------------------------
// VertexRing::InteriorPoint_ class.
// ----------------------------------------------------------------------------

//! An _InteriorPoint is created from an interior point of a bevel Profile.
struct VertexRing::InteriorPoint_ : public Point_ {
    InteriorPoint_(const Point3f &pos_in) : Point_(pos_in) {}
};

// ----------------------------------------------------------------------------
// VertexRing functions.
// ----------------------------------------------------------------------------

VertexRing::VertexRing(const Vertex &vertex, PolyMeshBuilder &builder,
                       size_t side_point_count) :
    vertex_(vertex),
    builder_(builder),
    side_point_count_(side_point_count) {
}

VertexRing::~VertexRing() {
    for (auto &pt: points_)
        delete pt;
}

void VertexRing::AddEdgeProfilePoints(const Edge &edge, bool is_reversed,
                                      std::vector<Point3f> edge_points) {
    BoundaryPoint_ *bp0;

    // If this is the first edge, create a new starting boundary point.
    if (points_.empty()) {
        bp0 = new BoundaryPoint_(*edge.face, nullptr, &edge, edge_points[0]);
        bp0->is_edge_reversed = is_reversed;
        points_.push_back(bp0);
    }
    else {
        // Otherwise, use the last added point, which must be a boundary point.
        bp0 = GetBoundaryPoint_(points_.size() - 1);

        // If the position of the first new point is not close enough, add an
        // auxiliary end point.
        if (! AreClose(bp0->pos, edge_points[0])) {
            bp0->end_point = new Point_(bp0->pos);
            bp0->pos       = edge_points[0];
        }
        bp0->start_edge       = &edge;
        bp0->is_edge_reversed = is_reversed;
    }

    // Add all interior points
    ASSERT(edge_points.size() >= 2U);
    for (size_t i = 1; i < edge_points.size() - 1; ++i)
        points_.push_back(new InteriorPoint_(edge_points[i]));

    // Create and add the last boundary point.
    ASSERT(edge.opposite_edge);
    points_.push_back(new BoundaryPoint_(*edge.opposite_edge->face, &edge,
                                         nullptr, edge_points.back()));

    ++edge_count_;
}

void VertexRing::EndEdges() {
    ASSERT(points_.size() > 2U);

    // Join the last boundary point to the first.
    BoundaryPoint_ *bp0 = GetBoundaryPoint_(0);
    BoundaryPoint_ *bp1 = GetBoundaryPoint_(points_.size() - 1);

    bp0->end_edge = bp1->end_edge;

    if (! AreClose(bp0->pos, bp1->pos))
        bp0->end_point = new Point_(bp1->pos);
    points_.pop_back();

    // See if the points enclose enough area to warrant adding triangles.
    has_area_ = ComputeArea(
        Util::ConvertVector<Point3f, Point_*>(
            points_, [](const Point_ *p){ return p->pos; })) > .01f;

    // If there is no area, merge points that are near to each other.
    if (! has_area_)
        MergePoints_();

    // Add all remaining points to the builder. These need to be added even
    // if the VertexRing has no area.
    AddPointsToBuilder_();

    // Sanity check. There is 1 boundary point and P-2 interior points for
    // each edge for a total of E * (P -1).
    ASSERT(edge_count_ * (side_point_count_ - 1) == points_.size());
}

VertexRing::IndexVec VertexRing::GetFacePointIndices(const Face &face) const {
    IndexVec indices;
    // Only boundary points can be in faces.
    for (auto &p: points_) {
        if (BoundaryPoint_ *bp = dynamic_cast<BoundaryPoint_*>(p)) {
            if (bp->face == &face) {
                if (bp->end_point)
                    indices.push_back(bp->end_point->index);
                indices.push_back(bp->index);
                break;
            }
        }
    }
    return indices;
}

VertexRing::IndexVec VertexRing::GetEdgePointIndices(const Edge &edge) const {
    IndexVec indices;

    auto find_edge_start = [&](const Edge &e){
        for (size_t i = 0; i < points_.size(); ++i) {
            BoundaryPoint_ *bp = dynamic_cast<BoundaryPoint_*>(points_[i]);
            if (bp && bp->start_edge == &e)
                return static_cast<int>(i);
        }
        return -1;
    };

    // Find the boundary point where the edge or its opposite edge starts.
    ASSERT(edge.opposite_edge);
    int start = find_edge_start(edge);
    if (start < 0)
        start = find_edge_start(*edge.opposite_edge);
    if (start >= 0) {
        // Add the starting boundary point and all interior points up to the
        // next boundary point,
        for (size_t i = 0; i < side_point_count_ - 1; ++i)
            indices.push_back(points_[InRange_(start + i)]->index);

        // Add the ending boundary point. Use its endPoint if it has one.
        BoundaryPoint_ *bp =
            GetBoundaryPoint_(InRange_(start + side_point_count_ - 1));
        indices.push_back(bp->end_point ? bp->end_point->index : bp->index);
    }
    return indices;
}

void VertexRing::AddPolygons(const Bevel &bevel) {
    // If there are only 2 or 3 profile points, no new internal points need to
    // be added.
    if (side_point_count_ <= 3U) {
        AddInnerPolygons_();
    }
    else {
        // Otherwise, create as many inner rings as necessary to get down to 3
        // or 2 points.
        std::vector<VertexRing *> rings;
        VertexRing *current_ring = this;
        while (true) {
            rings.push_back(current_ring);
            if (current_ring->side_point_count_ <= 3U)
                break;
            current_ring = current_ring->BuildInnerRing_();
        }

        // Use the bevel to position all of the points in the inner rings.
        RepositionRingPoints_(bevel, rings);

        // Add polygons between rings and inside the innermost ring.
        for (size_t i = 0; i < rings.size() - 1; ++i)
            rings[i]->AddPolygonsBetween_(*rings[i + 1]);

        // Innermost.
        rings.back()->AddInnerPolygons_();

        // Clean up.
        for (auto ring: rings)
            if (ring != this)
                delete ring;
    }
}

std::vector<Point3f> VertexRing::ApplyBevelToPoint(
    const Bevel &bevel, const Point3f &base_point,
    const Vector3f &vec0, const Vector3f &vec1) {

    const Vector3f sv0 = bevel.scale * vec0;
    const Vector3f sv1 = bevel.scale * vec1;

    const std::vector<Point3f> interior_pts =
        Util::ConvertVector<Point3f, Point2f>(
            bevel.profile.GetPoints(),
            [&base_point, &sv0, &sv1](const Point2f &p){
            return base_point + (1 - p[0]) * sv0 + (1 - p[1]) * sv1; });

    std::vector<Point3f> pts;
    pts.push_back(base_point + sv0);
    Util::AppendVector(interior_pts, pts);
    pts.push_back(base_point + sv1);
    return pts;
}

VertexRing * VertexRing::BuildInnerRing_() {
    VertexRing *inner_ring =
        new VertexRing(vertex_, builder_, side_point_count_ - 2);

    // Same number of edges.
    inner_ring->edge_count_ = edge_count_;

    // Create a new inner boundary point for each outer boundary point
    // (ignoring auxiliary end points). Create N-2 interior points for the N
    // outer interior points. The point positions do not matter; they are set
    // later.
    for (size_t i = 0; i < edge_count_; ++i) {
        const size_t bp_index = i * (side_point_count_ - 1);
        BoundaryPoint_ *bp = GetBoundaryPoint_(bp_index);
        inner_ring->points_.push_back(
            new BoundaryPoint_(*bp->face, bp->end_edge, bp->start_edge,
                               Point3f(0, 0, 0)));
        for (size_t j = 0; j < inner_ring->side_point_count_ - 2; ++j)
            inner_ring->points_.push_back(new InteriorPoint_(Point3f(0, 0, 0)));
    }
    inner_ring->AddPointsToBuilder_();

    return inner_ring;
}

void VertexRing::RepositionRingPoints_(const Bevel &bevel,
                                       const std::vector<VertexRing *> &rings) {
    // Use the outer ring (this, also first in the list) to create bevels used
    // to position all the points in the inner rings.
    ASSERT(rings[0] == this);

    // Strategy: given a pair of boundary points bp0/bp1, use the bevel to
    // position the inner ring points connecting the interior point before bp0
    // and the interior point after bp1. These points all lie on the next ring
    // in. Repeat with bp0-2 and bp1-2 and use the middle points of that to
    // position the points on the second ring in. Repeat until all rings are
    // done.

    // Iterate over all inner rings.
    for (size_t ring = 1; ring < rings.size(); ++ring) {
        size_t bp0_index = 0;
        for (size_t i = 0; i < edge_count_; ++i) {
            size_t bp1_index = InRange_(bp0_index + side_point_count_ - 1);
            BoundaryPoint_ *bp0 = GetBoundaryPoint_(bp0_index);

            // Get the end interior points used for the bevel's Profile.  The
            // ring index is also the offset from the current boundary points
            // to the interior points at the ends of the Profile.
            const size_t p0 = InRange_(bp0_index - ring);
            const size_t p1 = InRange_(bp1_index + ring);

            // Compute the bevel.
            const std::vector<Point3f> pts =
                ApplyBevel_(bevel, *bp0->start_edge, bp0->is_edge_reversed,
                            points_[p0]->pos, points_[p1]->pos);

            // Reposition points in the current inner ring.
            VertexRing *ivr = rings[ring];
            const size_t inner_start = i * (ivr->side_point_count_ - 1);
            for (size_t j = 0; j < ivr->side_point_count_; ++j)
                ivr->RepositionInteriorPoint_(ivr->InRange_(inner_start + j),
                                              pts[ring + j]);

            // Move to the next boundary point pair.
            bp0_index = bp1_index;
        }
    }
}

std::vector<Point3f> VertexRing::ApplyBevel_(
    const Bevel &bevel, const Edge &edge, bool is_reversed,
    const Point3f &end0, const Point3f &end1) {

    // If the endpoints are identical, just return a list of copies.
    if (end0 == end1)
        return std::vector<Point3f>(bevel.profile.GetPointCount(), end0);

    // Compute a base point on the edge for the profile. Use the closest point
    // on the edge to the line from end0 to end1.
    Point3f c0, c1;
    GetClosestLinePoints(edge.v0->point, edge.GetUnitVector(),
                         end0, ion::math::Normalized(end1 - end0), c0, c1);
    const Point3f base_point = c0;

    // Compute the vectors from the base point to the end point and scale them
    // so that applying the bevel scale reaches the end points:
    //     base_point + scale * vec0 = end0
    // SO
    //     vec0 = (end0 - base_point) / scale
    const Vector3f vec0 = (end0 - base_point) / bevel.scale;
    const Vector3f vec1 = (end1 - base_point) / bevel.scale;

    // Get the 3D profile.
    std::vector<Point3f> pts;
    if (is_reversed) {
        pts = ApplyBevelToPoint(bevel, base_point, vec1, vec0);
        std::reverse(pts.begin(), pts.end());
    }
    else {
        pts = ApplyBevelToPoint(bevel, base_point, vec0, vec1);
    }
    return pts;
}

void VertexRing::AddPolygonsBetween_(const VertexRing &inner_ring) {
    for (size_t i = 0; i < edge_count_; ++i) {
        // Get the starting (boundary) point for the edge on the outer and
        // inner rings.
        const size_t outer_start = i * (side_point_count_ - 1);
        const size_t inner_start = i * (inner_ring.side_point_count_ - 1);

        // Add the boundary corner quad.
        BoundaryPoint_ *bp = GetBoundaryPoint_(outer_start);
        Point_ *prev_pt    = points_[outer_start - 1];
        Point_ *next_pt    = points_[outer_start + 1];
        Point_ *inner_pt   = inner_ring.points_[inner_start];

        // Deal with end_point in boundary points.
        if (bp->end_point) {
            AddTriangle_(*prev_pt, *inner_pt, *bp->end_point);
            AddTriangle_(*bp->end_point, *inner_pt, *bp);
            AddTriangle_(*bp, *inner_pt, *next_pt);
        }
        else {
            AddQuad_(*inner_pt, *next_pt, *bp, *prev_pt);
        }

        // Add all interior quads.
        for (size_t j = 1; j < inner_ring.side_point_count_; ++j) {
            AddQuad_(*inner_ring.points_[inner_start + j - 1],
                     *inner_ring.points_[inner_start + j],
                     *points_[outer_start + j + 1],
                     *points_[outer_start + j]);
        }
    }
}


void VertexRing::AddInnerPolygons_() {
    ASSERT(side_point_count_ == 2U || side_point_count_ == 3U);

    // 2: Profiles are single edges. Just connect points into triangles,
    // dealing with any endPoints in boundary points. Use the first point added
    // in all triangles.
    if (side_point_count_ == 2U)
        AddCenterTriangles_();

    // 3: Profiles have one middle interior point. Connect them.
    else
        AddMiddlePolygons_();
}

void VertexRing::AddCenterTriangles_() {
    IndexVec indices;

    for (auto &p: points_) {
        // Add the index from the given Point_. If there are then 3 indices in
        // the list, add a triangle to the PolyMeshBuilder and remove the
        // second index from the list.
        indices.push_back(p->index);
        if (indices.size() == 3U) {
            // Profile vertex ordering has reverse orientation.
            builder_.AddTriangle(indices[2], indices[1], indices[0]);

            // Remove the second index.
            indices.erase(indices.begin() + 1);
        }
    }
}

void VertexRing::AddMiddlePolygons_() {
    IndexVec middle_points;

    for (size_t i = 0; i < edge_count_; ++i) {
        const size_t bp_index = i * (side_point_count_ - 1);
        BoundaryPoint_ *bp = GetBoundaryPoint_(bp_index);

        // Triangle or quad from boundary point to neighboring middle points.
        const Point_ *middle_point = points_[InRange_(bp_index + 1)];
        const Point_ *other_point  = points_[InRange_(bp_index - 1)];
        if (bp->end_point)
            AddQuad_(*bp, *bp->end_point, *other_point, *middle_point);
        else
            AddTriangle_(*bp, *other_point, *middle_point);
        middle_points.push_back(middle_point->index);
    }
    // Profile vertex ordering has reverse orientation.
    std::reverse(middle_points.begin(), middle_points.end());
    builder_.AddPolygon(middle_points);
}

void VertexRing::MergePoints_() {
    Point_ *prev_p = nullptr;
    for (auto &p: points_) {
        if (prev_p && ion::math::Distance(prev_p->pos, p->pos) <= .05f)
            p->pos = prev_p->pos;
        prev_p = p;
    }
}

void VertexRing::AddPointsToBuilder_() {
    for (auto &p: points_)
        p->index = builder_.AddVertex(p->pos);
}

void VertexRing::RepositionInteriorPoint_(size_t index,
                                          const Point3f &new_position) {
    ASSERT(index < points_.size());
    Point_ &p = *points_[index];
    if (++p.reposition_count == 1U) {
        p.pos = new_position;
    }
    else {
        // Should get repositioned twice (once for each bevel meeting
        // there). Use the average of the two positions.
        ASSERT(p.reposition_count == 2U);
        p.pos = .5f * (p.pos + new_position);
    }
    builder_.MoveVertex(p.index, p.pos);
}

void VertexRing::AddTriangle_(const Point_ &p0, const Point_ &p1,
                              const Point_ &p2) {
    builder_.AddTriangle(p0.index, p1.index, p2.index);
}

void VertexRing::AddQuad_(const Point_ &p0, const Point_ &p1,
                          const Point_ &p2, const Point_ &p3) {
    builder_.AddQuad(p0.index, p1.index, p2.index, p3.index);
}

VertexRing::BoundaryPoint_ * VertexRing::GetBoundaryPoint_(size_t index) const {
    BoundaryPoint_ *bp = dynamic_cast<BoundaryPoint_ *>(points_[index]);
    ASSERT(bp);
    return bp;
}
