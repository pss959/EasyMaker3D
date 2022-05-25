#include "Math/MeshBuilding.h"

#include <cmath>
#include <functional>
#include <vector>

#include "Math/Curves.h"
#include "Math/MeshCombining.h"
#include "Math/MeshUtils.h"
#include "Math/Polygon.h"
#include "Math/Profile.h"
#include "Math/Triangulation.h"
#include "Util/Assert.h"
#include "Util/General.h"

// ----------------------------------------------------------------------------
// Helper classes.
// ----------------------------------------------------------------------------

namespace {

/// Class that helps accumulate triangle indices.
class TriHelper_ {
  public:
    /// The constructor is given the vector of indices to fill in and the
    /// number of indices there will be, if known.
    TriHelper_(std::vector<GIndex> &indices, GIndex count) : indices_(indices) {
        indices_.reserve(count);
    }

    /// Adds a single triangle.
    void AddTri(GIndex i0, GIndex i1, GIndex i2) {
        indices_.push_back(i0);
        indices_.push_back(i1);
        indices_.push_back(i2);
    }

    /// Adds a quad; indices must be in circular order around the quad.
    void AddQuad(GIndex i0, GIndex i1, GIndex i2, GIndex i3,
                 bool reverse = false) {
        if (reverse) {
            AddTri(i0, i2, i1);
            AddTri(i0, i3, i2);
        }
        else {
            AddTri(i0, i1, i2);
            AddTri(i0, i2, i3);
        }
    }

    /// Adds a triangle fan.
    void AddFan(GIndex center, GIndex start, size_t count,
                bool wrap = false, bool reverse = false) {
        for (GIndex i = 0; i < count; ++i) {
            const GIndex i1 = start + i;
            const GIndex i2 = start + (wrap ? (i + 1) % count : i + 1);
            if (reverse)
                AddTri(center, i2, i1);
            else
                AddTri(center, i1, i2);
        }
    }

    /// Adds a grid of num_rows x num_cols points joined into quads.
    void AddGrid(GIndex start, size_t num_rows, size_t num_cols,
                 bool wrap = false, bool reverse = false) {
        const size_t row_offset = num_cols + (wrap ? 0 : 1);
        size_t top_row  = start;
        for (size_t r = 0; r < num_rows; ++r) {
            const size_t bot_row = top_row + row_offset;
            for (size_t c = 0; c < num_cols; ++c) {
                const size_t c_next = wrap ? (c + 1) % num_cols : c + 1;
                if (reverse)
                    AddQuad(bot_row + c_next, bot_row + c,
                            top_row + c,      top_row + c_next);
                else
                    AddQuad(top_row + c_next, top_row + c,
                            bot_row + c,      bot_row + c_next);
            }
            top_row = bot_row;
        }
    }

    /// Adds triangles from the given vector of indices, applying the given
    /// function to each one first and optionally reversing.
    void AddTris(const std::vector<GIndex> indices,
                 const std::function<GIndex(GIndex)> &func, bool reverse) {
        if (reverse) {
            for (auto it = indices.rbegin(); it != indices.rend(); ++it)
                indices_.push_back(func(*it));
        }
        else {
            for (GIndex i: indices)
                indices_.push_back(func(i));
        }
    }

  private:
    std::vector<GIndex> &indices_;
};

/// Class that helps with extruding Polygons.
class Extruder_ {
  public:
    explicit Extruder_(const Polygon &polygon);
    TriMesh Extrude(float height);

  private:
    const Polygon &polygon_;
    TriMesh        extruded_;
    TriHelper_     helper_;

    void AddPoints_(const Polygon &polygon, float height);
    void AddBorderSides_(size_t start, size_t count);

    static size_t GetRoughIndexCount_(const Polygon &polygon) {
        return (2 + 6 * polygon.GetHoleCount()) * polygon.GetPoints().size();
    }
};

Extruder_::Extruder_(const Polygon &polygon) :
    polygon_(polygon),
    helper_(extruded_.indices, GetRoughIndexCount_(polygon)) {
}

TriMesh Extruder_::Extrude(float height) {
    // Add points for the top and bottom polygons. Note that the Z coordinate
    // is negated to maintain the proper orientation for extruding in Y.
    AddPoints_(polygon_, height);

    // Triangulate the polygon.
    std::vector<GIndex> poly_tri_indices = TriangulatePolygon(polygon_);

    // Add the indices for the top and bottom polygons. Reverse the indices for
    // the bottom to maintain the proper orientation.
    const size_t poly_size = polygon_.GetPoints().size();
    auto top_func =          [](GIndex i){ return i; };
    auto bot_func = [poly_size](GIndex i){ return poly_size + i; };
    helper_.AddTris(poly_tri_indices, top_func, false);
    helper_.AddTris(poly_tri_indices, bot_func, true);

    // Add sides for the outer border.
    const std::vector<size_t> &counts = polygon_.GetBorderCounts();
    AddBorderSides_(0, counts[0]);

    // Do the same for the holes. Reverse the order for correct orientation.
    for (size_t i = 1; i < counts.size(); ++i)
        AddBorderSides_(polygon_.GetHoleStartIndex(i - 1), counts[i]);

    return extruded_;
}

void Extruder_::AddBorderSides_(size_t start, size_t count) {
    const size_t poly_size = polygon_.GetPoints().size();
    for (size_t i = 1; i < count; ++i) {
        const GIndex top = start + i;
        const GIndex bot = top + poly_size;
        helper_.AddQuad(top, top - 1, bot - 1, bot);
    }
    // Connect last to first.
    helper_.AddQuad(start, start + count - 1,
                    start + poly_size + count - 1, start + poly_size);
}

void Extruder_::AddPoints_(const Polygon &polygon, float height) {
    const std::vector<Point3f> top_pts =
        Util::ConvertVector<Point3f, Point2f>(
            polygon.GetPoints(),
            [height](const Point2f &p){ return Point3f(p[0], height, -p[1]); });
    const std::vector<Point3f> bot_pts =
        Util::ConvertVector<Point3f, Point3f>(
            top_pts, [](const Point3f &p){ return Point3f(p[0], 0, p[2]); });
    Util::AppendVector(top_pts, extruded_.points);
    Util::AppendVector(bot_pts, extruded_.points);
}

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

/// Adds points forming a circular arc in the Y=y plane with the given radius
/// to the given vector of points.
static void AddArcPoints_(const std::vector<Point2f> arc_pts,
                          float rad, float y,
                          std::vector<Point3f> &mesh_pts) {
    for (const auto &p: arc_pts)
        mesh_pts.push_back(Point3f(rad * p[0], y, rad * p[1]));
}

/// Builds a surface of revolution that has a sweepAngle of 360 degrees.
static TriMesh BuildFullRevSurf_(const Profile &profile, int num_sides) {
    TriMesh mesh;

     // P is the number of interior Profile points (always >= 1).
    const size_t p = profile.GetPoints().size();

    // There is 1 vertex at the top center, 1 at the bottom center, and
    // p*num_sides in the middle.
    const size_t point_count = 2 + p * num_sides;
    mesh.points.reserve(point_count);
    const std::vector<Point2f> ring_pts = GetCirclePoints(num_sides, 1, true);
    mesh.points.push_back(Point3f(profile.GetStartPoint(), 0));
    for (const auto &pp: profile.GetPoints())
        AddArcPoints_(ring_pts, pp[0], pp[1], mesh.points);
    mesh.points.push_back(Point3f(profile.GetEndPoint(), 0));
    ASSERT(mesh.points.size() == point_count);

    // Translate all mesh points down in Y by .5 to center them.
    for (auto &p: mesh.points)
        p[1] -= .5f;

    // There are num_sides triangles in each of the top and bottom fans and
    // 2*(p-1)*num_sides triangles around the sides.
    const size_t index_count = 3 * ((2 + 2 * (p - 1)) * num_sides);
    TriHelper_ helper(mesh.indices, index_count);
    const GIndex top_index    = 0;
    const GIndex bottom_index = mesh.points.size() - 1;
    helper.AddFan(top_index, top_index + 1, num_sides, true);
    helper.AddGrid(top_index + 1, p - 1, num_sides, true);
    helper.AddFan(bottom_index, bottom_index - num_sides, num_sides,
                  true, true);
    ASSERT(mesh.indices.size() == index_count);

    return mesh;
 }

/// Builds a surface of revolution that has a sweep angle of less than 360
/// degrees.
static TriMesh BuildPartialRevSurf_(const Profile &profile,
                                    const Anglef &sweep_angle, int num_sides) {
    // Consider a circular arc subtending the sweep angle and containing C
    // points. If there are p (>= 1) points in the interior of the Profile,
    // then the unwrapped center part is a grid that is c-1 quads wide and p-1
    // quads high. Each quad is 2 triangles.
    //
    // The top and bottom pieces are fans consisting of C-1 triangles each,
    // with the fixed Profile point at the corresponding end in each triangle.
    TriMesh mesh;

    // Determine C, the number of circular arc points, and P, the number of
    // interior profile points. Get the points forming the circular arc.
    const float sweep_fraction = sweep_angle.Degrees() / 360.f;
    const size_t c = std::max(2, 1 + static_cast<int>(sweep_fraction *
                                                      num_sides));
    const size_t p = profile.GetPoints().size();
    const std::vector<Point2f> arc_pts =
        GetCircleArcPoints(c, 1, CircleArc(Anglef(), sweep_angle));

    // There is 1 vertex at the top center, 1 at the bottom center, and p*c
    // vertices in the middle.
    const size_t point_count = 2 + p * c;
    mesh.points.reserve(point_count);
    mesh.points.push_back(Point3f(profile.GetStartPoint(), 0));
    for (const auto &pp: profile.GetPoints())
        AddArcPoints_(arc_pts, pp[0], pp[1], mesh.points);
    mesh.points.push_back(Point3f(profile.GetEndPoint(), 0));
    ASSERT(mesh.points.size() == point_count);

    // Translate all mesh points down in Y by .5 to center them.
    for (auto &p: mesh.points)
        p[1] -= .5f;

    // Create a Polygon with all profile points and triangulate it.
    Polygon poly(profile.GetAllPoints());
    std::vector<GIndex> poly_tri_indices = TriangulatePolygon(poly);
    const size_t poly_tri_count = poly_tri_indices.size() / 3;

    // There are c-1 triangles in each of the top and bottom fans and
    // 2*(p-1)*(c*1) triangles around the sides. Total is 2*p*(c-1) plus 2
    // times the poly_tri_count for the end caps.
    const size_t index_count = 3 * (2 * p * (c - 1) + 2 * poly_tri_count);
    TriHelper_ helper(mesh.indices, index_count);
    const GIndex top_index    = 0;
    const GIndex bottom_index = mesh.points.size() - 1;
    helper.AddFan(top_index, top_index + 1, c - 1, false, true);
    helper.AddGrid(top_index + 1, p - 1, c - 1, false, true);
    helper.AddFan(bottom_index, bottom_index - c, c - 1);

    // These return the index of the vertex at the start or end of the given
    // row. Row indexing is 0 for the top point, and so on.
    auto row_start = [c](size_t row){
        return row == 0 ? 0 : 1 + (row - 1) * c; };
    auto row_end   = [c, p, bottom_index, row_start](size_t row){
        return row == 0 ? 0 :
            (row == p + 1 ? bottom_index : row_start(row) + c - 1); };

    // Start and end cap polygons. For index 0, use the top point. For all
    // other indices, use the first point in the row with that index-1.
    helper.AddTris(poly_tri_indices, row_start, true);
    helper.AddTris(poly_tri_indices, row_end,   false);
    ASSERT(mesh.indices.size() == index_count);

    return mesh;
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

TriMesh BuildTetrahedronMesh(float size) {
    const float hs = .5f * size;

    TriMesh mesh;
    mesh.points.assign({
            Point3f(-hs, -hs, -hs),     // Base back left.
            Point3f( hs, -hs, -hs),     // Base back right.
            Point3f(  0, -hs,  hs),     // Base front center.
            Point3f(  0,  hs,   0) });  // Apex.
    mesh.indices.assign({
            0, 1, 2,     // Base
            0, 3, 1,     // Front
            0, 2, 3,     // Back left
            1, 3, 2 });  // Back right

    // No need to clean this mesh.
    return mesh;
}

TriMesh BuildBoxMesh(const Vector3f &size) {
    const float w = .5f * size[0];
    const float h = .5f * size[1];
    const float d = .5f * size[2];

    TriMesh mesh;
    mesh.points.assign({
            Point3f(-w, -h, -d),
            Point3f(-w, -h,  d),
            Point3f(-w,  h, -d),
            Point3f(-w,  h,  d),
            Point3f( w, -h, -d),
            Point3f( w, -h,  d),
            Point3f( w,  h, -d),
            Point3f( w,  h,  d),
        });
    mesh.indices.assign({
            0, 1, 2,   1, 3, 2,
            2, 3, 6,   3, 7, 6,
            4, 6, 5,   7, 5, 6,
            0, 4, 5,   1, 0, 5,
            1, 5, 7,   1, 7, 3,
            0, 2, 6,   0, 6, 4,
        });

    // No need to clean this mesh.
    return mesh;
}

TriMesh BuildCylinderMesh(float top_radius, float bottom_radius,
                          float height, int num_sides) {
    const float h = .5f * height;

    TriMesh mesh;

    // There is 1 point at the top center, 1 at the bottom center, and
    // 2*num_sides forming the top and bottom edge rings.
    const size_t point_count = 2 + 2 * num_sides;
    const std::vector<Point2f> ring_pts = GetCirclePoints(num_sides, 1, true);
    mesh.points.push_back(Point3f(0, h, 0));
    AddArcPoints_(ring_pts,    top_radius,  h, mesh.points);
    AddArcPoints_(ring_pts, bottom_radius, -h, mesh.points);
    mesh.points.push_back(Point3f(0, -h, 0));
    ASSERT(mesh.points.size() == point_count);

    // There are num_sides triangles each in the top and bottom fans and
    // 2*num_sides triangles around the sides.
    const size_t index_count = 3 * (4 * num_sides);
    TriHelper_ helper(mesh.indices, index_count);
    const GIndex top_index    = 0;
    const GIndex bottom_index = mesh.points.size() - 1;
    helper.AddFan(top_index, top_index + 1, num_sides, true);
    helper.AddGrid(top_index + 1, 1, num_sides, true);
    helper.AddFan(bottom_index, bottom_index - num_sides, num_sides,
                  true, true);
    ASSERT(mesh.indices.size() == index_count);

    CleanMesh(mesh);
    return mesh;
}

TriMesh BuildRevSurfMesh(const Profile &profile, const Anglef &sweep_angle,
                         int num_sides) {
    const float angle = sweep_angle.Degrees();
    ASSERT(angle > 0 && angle <= 360);
    TriMesh mesh = angle == 360 ?
        BuildFullRevSurf_(profile, num_sides) :
        BuildPartialRevSurf_(profile, sweep_angle, num_sides);
    CleanMesh(mesh);
    return mesh;
}

TriMesh BuildSphereMesh(float radius, int num_rings, int num_sectors) {
    TriMesh mesh;

    // Precompute a ring of radius 1 vertices for speed.
    const std::vector<Point2f> ring_pts = GetCirclePoints(num_sectors, 1, true);

    // There is 1 point at the top, 1 at the bottom, and num_rings*num_sectors
    // in the middle.
    const size_t point_count = 2 + num_rings * num_sectors;
    mesh.points.reserve(point_count);
    // Top point.
    mesh.points.push_back(Point3f(0, radius, 0));
    // Rings.
    const Anglef delta_z_angle  = Anglef::FromDegrees(180) / (num_rings + 1);
    for (int r = 0; r < num_rings; ++r) {
        const Anglef y_angle =
            Anglef::FromDegrees(90.f) - (r + 1) * delta_z_angle;
        const float ring_radius = ion::math::Cosine(y_angle) * radius;
        const float ring_y      = ion::math::Sine(y_angle);
        AddArcPoints_(ring_pts, ring_radius, ring_y, mesh.points);
    }
    // Bottom point.
    mesh.points.push_back(Point3f(0, -radius, 0));
    ASSERT(mesh.points.size() == point_count);

    // There are num_sectors triangles in each fan around the top and bottom
    // points + 2*num_sectors triangles in each of the bands between rings.
    const int num_bands = num_rings - 1;
    const size_t index_count = 3 * ((2 + 2 * num_bands) * num_sectors);
    TriHelper_ helper(mesh.indices, index_count);
    const GIndex top_index    = 0;
    const GIndex bottom_index = mesh.points.size() - 1;
    helper.AddFan(top_index, top_index + 1, num_sectors, true);
    helper.AddGrid(top_index + 1, num_bands, num_sectors, true);
    helper.AddFan(bottom_index, bottom_index - num_sectors, num_sectors,
                  true, true);
    ASSERT(mesh.indices.size() == index_count);

    CleanMesh(mesh);
    return mesh;
}

TriMesh BuildTorusMesh(float inner_radius, float outer_radius,
                       int num_rings, int num_sectors) {
    TriMesh mesh;

    // Precompute points in a ring. Convert to 3D so that the ring is in the XY
    // plane and offset the points so that the center of the ring reaches
    // (outer_radius - inner_radius) in X.
    const float ring_radius = outer_radius - inner_radius;
    const std::vector<Point2f> ring_pts =
        GetCirclePoints(num_rings, inner_radius, true);
    std::vector<Point3f> ring_pts_3d;
    ring_pts_3d.reserve(num_rings);
    for (int i = 0; i < num_rings; ++i)
        ring_pts_3d.push_back(Point3f(ring_pts[i][0] + ring_radius,
                                      ring_pts[i][1], 0));

    // Iterate over all sectors, rotating the ring_vertices into position.
    const size_t point_count = num_rings * num_sectors;
    mesh.points.reserve(point_count);
    for (int s = 0; s < num_sectors; ++s) {
        const Anglef angle = Anglef::FromDegrees(s * 360.f / num_sectors);
        const Rotationf rot =
            Rotationf::FromAxisAndAngle(Vector3f::AxisY(), angle);
        for (int r = 0; r < num_rings; ++r)
            mesh.points.push_back(rot * ring_pts_3d[r]);
    }
    ASSERT(mesh.points.size() == point_count);

    // There are 2 triangles for each quad of rings/sectors.
    const size_t index_count = 3 * (2 * num_rings * num_sectors);
    TriHelper_ helper(mesh.indices, index_count);
    int first_ring_index = 0;
    for (int s = 0; s < num_sectors; ++s) {
        const int next_ring_index =
            (first_ring_index + num_rings) % (num_rings * num_sectors);
        for (int r = 0; r < num_rings; ++r) {
            helper.AddQuad(first_ring_index + r,
                           first_ring_index + (r + 1) % num_rings,
                           next_ring_index  + (r + 1) % num_rings,
                           next_ring_index  + r);
        }
        first_ring_index += num_rings;
    }
    ASSERT(mesh.indices.size() == index_count);

    CleanMesh(mesh);
    return mesh;
}

TriMesh BuildPolygonMesh(const Polygon &polygon) {
    // Convert 2D points to 3D and triangulate to get indices.
    TriMesh mesh;
    mesh.points = Util::ConvertVector<Point3f, Point2f>(
        polygon.GetPoints(), [](const Point2f &p){ return Point3f(p, 0); });
    mesh.indices = TriangulatePolygon(polygon);
    return mesh;
}

TriMesh BuildExtrudedMesh(const Polygon &polygon, float height) {
    return Extruder_(polygon).Extrude(height);
}
