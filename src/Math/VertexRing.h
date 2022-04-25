#pragma once

#include <vector>

#include "Math/Bevel.h"
#include "Math/PolyMesh.h"
#include "Math/PolyMeshBuilder.h"
#include "Math/Types.h"

/// VertexRing is used internally by the Beveler class to manage a ring of
/// points created by applying a Profile to PolyMesh edges meeting at a vertex.
///
/// \ingroup Math
class VertexRing {
  public:
    typedef PolyMesh::Edge     Edge;      ///< Convenience typedef.
    typedef PolyMesh::Face     Face;      ///< Convenience typedef.
    typedef PolyMesh::Vertex   Vertex;    ///< Convenience typedef.
    typedef PolyMesh::IndexVec IndexVec;  ///< Convenience typedef.

    /// The constructor is passed the original PolyMesh::Vertex the ring
    /// represents, the PolyMeshBuilder used to construct the final mesh, and
    /// the number of points on each side of the ring.
    VertexRing(const Vertex &vertex, PolyMeshBuilder &builder,
               size_t side_point_count);

    ~VertexRing();

    /// Adds points corresponding to a bevel profile for an edge. The
    /// is_reversed flag indicates whether the profiles should be reversed for
    /// the edge.
    void AddEdgeProfilePoints(const Edge &edge, bool is_reversed,
                              std::vector<Point3f> edge_points);

    /// This should be called after all edges have been added to close the
    /// ring.
    void EndEdges();

    /// Returns the number of points in the ring.
    size_t GetPointCount() const { return points_.size(); }

    /// Returns true if the VertexRing encloses an area, meaning that not all
    /// points are collinear. This is useful only after EndEdges() is called.
    bool HasArea() const { return has_area_; }

    /// Returns indices of all ring points that are in the given face. There
    /// will be either 1 or 2 points.
    IndexVec GetFacePointIndices(const Face &face) const;

    /// Returns indices of all consecutive ring points that came from the given
    /// edge or its opposite.
    IndexVec GetEdgePointIndices(const Edge &edge) const;

    /// Adds polygons to the PolyMeshBuilder to fill in the ring. The Bevel is
    /// supplied for creating new points.
    void AddPolygons(const Bevel &bevel);

    /// Creates and returns a 3D profile (vector of Point3f instances) by
    /// applying a Bevel to the given base point (on the edge) and endpoint
    /// vectors. The first profile point at (0,1) will end up at
    /// base_point+scale*vec0, and the last profile point at (1,0) will end up
    /// at base_point+scale*vec1.
    static std::vector<Point3f> ApplyBevelToPoint(
        const Bevel &bevel, const Point3f &base_point,
        const Vector3f &vec0, const Vector3f &vec1);

  private:
    struct BoundaryPoint_;
    struct InteriorPoint_;
    struct Point_;

    /// Vertex the VertexRing represents.
    const Vertex &vertex_;

    /// PolyMeshBuilder used to add points and polygons.
    PolyMeshBuilder &builder_;

    /// Number of points in one side of the ring, which equals the number of
    /// points between a pair of boundary points, inclusive.
    const size_t side_point_count_;

    /// Points forming the ring. Ring points travel clockwise when viewed from
    /// outside the vertex.
    std::vector<Point_ *> points_;

    /// This is set to false if all added points are collinear, meaning that
    /// the VertexRing does not enclose an area, so there are no valid
    /// triangles to add.
    bool has_area_ = false;

    /// Number of edges added to the ring.
    size_t edge_count_ = 0;

    /// Builds and returns a new inner VertexRing.
    VertexRing * BuildInnerRing_();

    /// Computes new positions for all points in inner rings using bevel curves
    /// between points on the outer ring. The rings list contains all
    /// VertexRings in order from outermost to innermost.
    void RepositionRingPoints_(const Bevel &bevel,
                               const std::vector<VertexRing *> &rings);

    /// Returns the points resulting from applying a Bevel to an edge.
    std::vector<Point3f> ApplyBevel_(const Bevel &bevel, const Edge &edge,
                                     bool is_reversed,
                                     const Point3f &end0, const Point3f &end1);

    /// Adds polygons between this ring and the next inner ring.
    void AddPolygonsBetween_(const VertexRing &inner_ring);

    /// Adds polygons to fill in a ring that has either 2 or 3 points per edge,
    /// meaning that no new rings or points need to be created.
    void AddInnerPolygons_();

    /// Adds triangles to fill in the center of a ring with 2 points per
    /// profile.
    void AddCenterTriangles_();

    /// Adds polygons for the case where there is exactly 1 interior point
    /// in each profile, meaning that no new points need to be created.
    void AddMiddlePolygons_();

    /// This is used for a VertexRing with zero or very small area. It merges
    /// the locations of neighboring points that are close enough together so
    /// that there are fewer anomalous cases.
    void MergePoints_();

    /// Adds all points in the ring to the PolyMeshBuilder, storing indices.
    void AddPointsToBuilder_();

    /// Changes the position of an interior point.
    void RepositionInteriorPoint_(size_t index, const Point3f &new_position);

    /// Convenience to add a triangle to the PolyMeshBuilder.
    void AddTriangle_(const Point_ &p0, const Point_ &p1, const Point_ &p2);

    /// Convenience to add a quad from indexed Point_ instances to the
    /// PolyMeshBuilder.
    void AddQuad_(const Point_ &p0, const Point_ &p1,
                  const Point_ &p2, const Point_ &p3);

    /// Convenience that casts the indexed Point_ to a BoundaryPoint_ and
    /// asserts that it worked.
    BoundaryPoint_ * GetBoundaryPoint_(size_t index) const;

    /// Convenience that returns the point at the given index, calling
    /// InRange_() first.
    Point_ * GetPoint_(int index) const {
        return points_[InRange_(index)];
    }
    /// Convenience that returns an index into points_ wrapped to be in range
    /// if necessary.
    size_t InRange_(int index) const {
        return index < 0 ? index + points_.size() : index % points_.size();
    }
};
