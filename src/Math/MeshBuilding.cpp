#include "Math/MeshBuilding.h"

#include <cmath>
#include <functional>

#include "Math/CGALInterface.h"
#include "Math/Curves.h"
#include "Math/Polygon.h"
#include "Math/Profile.h"

// ----------------------------------------------------------------------------
// Helper classes.
// ----------------------------------------------------------------------------

//! Class that helps accumulate triangle indices.
class TriHelper_ {
  public:
    //! The constructor is given the vector of indices to fill in and the
    //! number of indices there will be.
    TriHelper_(std::vector<int> &indices, size_t count) : indices_(indices) {
        indices_.reserve(count);
    }
    //! Adds a single triangle.
    void AddTri(int i0, int i1, int i2) {
        indices_.push_back(i0);
        indices_.push_back(i1);
        indices_.push_back(i2);
    }

    //! Adds a quad; indices must be in circular order around the quad.
    void AddQuad(int i0, int i1, int i2, int i3) {
        AddTri(i0, i1, i2);
        AddTri(i0, i2, i3);
    }

    //! Adds a triangle fan.
    void AddFan(int center, int start, int count,
                bool wrap = false, bool reverse = false) {
        for (int i = 0; i < count; ++i) {
            int i1 = start + i;
            int i2 = start + (wrap ? (i + 1) % count : i + 1);
            if (reverse)
                AddTri(center, i2, i1);
            else
                AddTri(center, i1, i2);
        }
    }

    //! Adds a grid of num_rows x num_cols points joined into quads.
    void AddGrid(int start, int num_rows, int num_cols,
                 bool wrap = false, bool reverse = false) {
        const int row_offset = num_cols + (wrap ? 0 : 1);
        int top_row  = start;
        for (int r = 0; r < num_rows; ++r) {
            int bot_row = top_row + row_offset;
            for (int c = 0; c < num_cols; ++c) {
                int c_next = wrap ? (c + 1) % num_cols : c + 1;
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

    //! Adds triangles from the given vector of indices, applying the given
    //! function to each one first and optionally reversing.
    void AddTris(const std::vector<size_t> indices,
                 const std::function<size_t(size_t)> &func, bool reverse) {
        if (reverse) {
            for (auto it = indices.rbegin(); it != indices.rend(); ++it)
                indices_.push_back(func(*it));
        }
        else {
            for (int i: indices)
                indices_.push_back(func(i));
        }
    }

  private:
    std::vector<int> &indices_;
};

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

//! Adds points forming a circular arc in the Y=y plane with the given radius
//! to the given vector of points.
static void AddArcPoints_(const std::vector<Point2f> arc_pts,
                          float rad, float y,
                          std::vector<Point3f> &mesh_pts) {
    for (const auto &p: arc_pts)
        mesh_pts.push_back(Point3f(rad * p[0], y, rad * p[1]));
}

//! Builds a surface of revolution that has a sweepAngle of 360 degrees.
static TriMesh BuildFullRevSurf_(const Profile &profile, int num_sides) {
    TriMesh mesh;

     // P is the number of interior Profile points (always >= 1).
    const size_t p = profile.GetPoints().size();

    // There is 1 vertex at the top center, 1 at the bottom center, and
    // p*num_sides in the middle.
    const size_t point_count = 2 + p * num_sides;
    mesh.points.reserve(point_count);
    const std::vector<Point2f> ring_pts = GetCirclePoints(num_sides, 1);
    mesh.points.push_back(Point3f(profile.GetStartPoint(), 0));
    for (const auto &pp: profile.GetPoints())
        AddArcPoints_(ring_pts, pp[0], pp[1], mesh.points);
    mesh.points.push_back(Point3f(profile.GetEndPoint(), 0));
    ASSERT(mesh.points.size() == point_count);

    // There are num_sides triangles in each of the top and bottom fans and
    // 2*(p-1)*num_sides triangles around the sides.
    const size_t index_count = 3 * ((2 + 2 * (p - 1)) * num_sides);
    TriHelper_ helper(mesh.indices, index_count);
    const size_t top_index    = 0;
    const size_t bottom_index = mesh.points.size() - 1;
    helper.AddFan(top_index, top_index + 1, num_sides, true);
    helper.AddGrid(top_index + 1, p - 1, num_sides, true);
    helper.AddFan(bottom_index, bottom_index - num_sides, num_sides,
                  true, true);
    ASSERT(mesh.indices.size() == index_count);

    return mesh;
 }

//! Builds a surface of revolution that has a sweep angle of less than 360
//! degrees.
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
        GetCircleArcPoints(c, 1, Anglef(), sweep_angle);

    // There is 1 vertex at the top center, 1 at the bottom center, and p*c
    // vertices in the middle.
    const size_t point_count = 2 + p * c;
    mesh.points.reserve(point_count);
    mesh.points.push_back(Point3f(profile.GetStartPoint(), 0));
    for (const auto &pp: profile.GetPoints())
        AddArcPoints_(arc_pts, pp[0], pp[1], mesh.points);
    mesh.points.push_back(Point3f(profile.GetEndPoint(), 0));
    ASSERT(mesh.points.size() == point_count);

    // Create a Polygon with all profile points and triangulate it.
    Polygon poly(profile.GetAllPoints());
    std::vector<size_t> poly_tri_indices = TriangulatePolygon(poly);
    const size_t poly_tri_count = poly_tri_indices.size() / 3;

    // There are c-1 triangles in each of the top and bottom fans and
    // 2*(p-1)*(c*1) triangles around the sides. Total is 2*p*(c-1) plus 2
    // times the poly_tri_count for the end caps.
    const size_t index_count = 3 * (2 * p * (c - 1) + 2 * poly_tri_count);
    TriHelper_ helper(mesh.indices, index_count);
    const size_t top_index    = 0;
    const size_t bottom_index = mesh.points.size() - 1;
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
    return mesh;
}

TriMesh BuildCylinderMesh(float top_radius, float bottom_radius,
                          float height, int num_sides) {
    const float h = .5f * height;

    TriMesh mesh;

    // There is 1 point at the top center, 1 at the bottom center, and
    // 2*num_sides forming the top and bottom edge rings.
    const size_t point_count = 2 + 2 * num_sides;
    const std::vector<Point2f> ring_pts = GetCirclePoints(num_sides, 1);
    mesh.points.push_back(Point3f(0, h, 0));
    AddArcPoints_(ring_pts,    top_radius,  h, mesh.points);
    AddArcPoints_(ring_pts, bottom_radius, -h, mesh.points);
    mesh.points.push_back(Point3f(0, -h, 0));
    ASSERT(mesh.points.size() == point_count);

    // There are num_sides triangles each in the top and bottom fans and
    // 2*num_sides triangles around the sides.
    const size_t index_count = 3 * (4 * num_sides);
    TriHelper_ helper(mesh.indices, index_count);
    const size_t top_index    = 0;
    const size_t bottom_index = mesh.points.size() - 1;
    helper.AddFan(top_index, top_index + 1, num_sides, true);
    helper.AddGrid(top_index + 1, 1, num_sides, true);
    helper.AddFan(bottom_index, bottom_index - num_sides, num_sides,
                  true, true);
    ASSERT(mesh.indices.size() == index_count);

    return mesh;
}

TriMesh BuildRevSurfMesh(const Profile &profile, const Anglef &sweep_angle,
                         int num_sides) {
    const float angle = sweep_angle.Degrees();
    ASSERT(angle > 0 && angle <= 360);
    if (angle == 360)
        return BuildFullRevSurf_(profile, num_sides);
    else
        return BuildPartialRevSurf_(profile, sweep_angle, num_sides);
}

TriMesh BuildSphereMesh(float radius, int num_rings, int num_sectors) {
    TriMesh mesh;

    // Precompute a ring of radius 1 vertices for speed.
    const std::vector<Point2f> ring_pts = GetCirclePoints(num_sectors, 1);

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
    const size_t top_index    = 0;
    const size_t bottom_index = mesh.points.size() - 1;
    helper.AddFan(top_index, top_index + 1, num_sectors, true);
    helper.AddGrid(top_index + 1, num_bands, num_sectors, true);
    helper.AddFan(bottom_index, bottom_index - num_sectors, num_sectors,
                  true, true);
    ASSERT(mesh.indices.size() == index_count);

    return mesh;
}

TriMesh BuildTorusMesh(float inner_radius, float outer_radius,
                       int num_rings, int num_sectors) {
    TriMesh mesh;

    // Precompute points in a ring. Convert to 3D so that the ring is in the XY
    // plane and offset the points so that the outside of the ring reaches
    // outer_radius in X.
    const std::vector<Point2f> ring_pts =
        GetCirclePoints(num_rings, inner_radius);
    std::vector<Point3f> ring_pts_3d;
    ring_pts_3d.reserve(num_rings);
    for (int i = 0; i < num_rings; ++i)
        ring_pts_3d.push_back(
            Point3f(ring_pts[i][0] + outer_radius - inner_radius,
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

    return mesh;
}
