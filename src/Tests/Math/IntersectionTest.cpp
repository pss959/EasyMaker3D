#include "Math/Bounds.h"
#include "Math/Intersection.h"
#include "Math/MeshBuilding.h"
#include "Math/Plane.h"
#include "Math/Ray.h"
#include "Math/TriMesh.h"
#include "Math/Types.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class IntersectionTest : public TestBase {};

TEST_F(IntersectionTest, Ray) {
    const Ray r0(Point3f(-4, -3, 6), Vector3f(-1, 0, 0));
    const Ray r1(Point3f(-4, -3, 6), Vector3f(-1, 0, 0));
    const Ray r2(Point3f(-4, -3, 6), Vector3f( 1, 0, 0));
    EXPECT_EQ(r0, r0);
    EXPECT_EQ(r0, r1);
    EXPECT_NE(r0, r2);
}

TEST_F(IntersectionTest, RayBoundsIntersect) {
    // Also tests RayBoundsIntersectFace().

    float dist;
    const Bounds b(Point3f(-4, -3, 6), Point3f(-2, 9, 12));

    const auto intersect = [&](float x, float y, float z, const Vector3f &dir){
        return RayBoundsIntersect(Ray(Point3f(x, y, z), dir), b, dist);
    };

    EXPECT_TRUE(intersect(-3, 3, 22, -Vector3f::AxisZ()));
    EXPECT_CLOSE(10.f, dist);
    EXPECT_TRUE(intersect(-3, 3, -4,  Vector3f::AxisZ()));
    EXPECT_CLOSE(10.f, dist);
    EXPECT_TRUE(intersect(-14, 4, 8,  Vector3f::AxisX()));
    EXPECT_CLOSE(10.f, dist);
    EXPECT_TRUE(intersect(8,   4, 8, -Vector3f::AxisX()));
    EXPECT_CLOSE(10.f, dist);

    // Close misses.
    EXPECT_FALSE(intersect(-4.1f, 3,    22, -Vector3f::AxisZ()));
    EXPECT_FALSE(intersect(-3,   -3.1f, 22, -Vector3f::AxisZ()));

    // Start inside or past bounds.
    EXPECT_FALSE(intersect(-3,    3,    10, -Vector3f::AxisZ()));
    EXPECT_FALSE(intersect(-3,    3,     4, -Vector3f::AxisZ()));

    // Bad ray.
    EXPECT_FALSE(intersect(-3, 3, 9, Vector3f::Zero()));
}

TEST_F(IntersectionTest, RayPlaneIntersect) {
    float dist;
    const auto intersect = [&](const Plane &p, float x, float y, float z,
                               const Vector3f &dir){
        return RayPlaneIntersect(Ray(Point3f(x, y, z), dir), p, dist);
    };

    const Plane p0(0, Vector3f::AxisZ());
    EXPECT_TRUE(intersect(p0, 0, 0, 10, -Vector3f::AxisZ()));
    EXPECT_CLOSE(10.f, dist);
    EXPECT_TRUE(intersect(p0, 100, -100, 10, -Vector3f::AxisZ()));
    EXPECT_CLOSE(10.f, dist);

    const Plane p1(10, Vector3f::AxisX());
    EXPECT_TRUE(intersect(p1, 20, 0, 0, -Vector3f::AxisX()));
    EXPECT_CLOSE(10.f, dist);
    EXPECT_TRUE(intersect(p1, -40, -100, 100, Vector3f::AxisX()));
    EXPECT_CLOSE(50.f, dist);

    // Wrong side.
    EXPECT_FALSE(intersect(p0, 0, 0, 10, Vector3f::AxisZ()));

    // Bad ray.
    EXPECT_FALSE(intersect(p0, 10, 10, 20, Vector3f::Zero()));
}

TEST_F(IntersectionTest, RayTriMeshIntersect) {
    // Also tests RayTriangleIntersect().

    float        dist;
    TriMesh::Hit hit;
    const TriMesh box = BuildBoxMesh(Vector3f(40, 20, 10));

    const auto intersect = [&](float x, float y, float z, const Vector3f &dir){
        return RayTriMeshIntersect(Ray(Point3f(x, y, z), dir), box, dist, hit);
    };

    EXPECT_TRUE(intersect(0, 0, 15, -Vector3f::AxisZ()));
    EXPECT_CLOSE(10.f, dist);
    EXPECT_PTS_CLOSE(Point3f(0, 0, 5),       hit.point);
    EXPECT_VECS_CLOSE(Vector3f(0, 0, 1),     hit.normal);
    EXPECT_EQ(Vector3i(1, 5, 7),             hit.indices);
    EXPECT_VECS_CLOSE(Vector3f(.5f, 0, .5f), hit.barycentric);

    EXPECT_TRUE(intersect(-30, 5, 2, Vector3f::AxisX()));
    EXPECT_CLOSE(10.f, dist);
    EXPECT_PTS_CLOSE(Point3f(-20, 5, 2),         hit.point);
    EXPECT_VECS_CLOSE(Vector3f(-1, 0, 0),        hit.normal);
    EXPECT_EQ(Vector3i(1, 3, 2),                 hit.indices);
    EXPECT_VECS_CLOSE(Vector3f(.25f, .45f, .3f), hit.barycentric);

    // Inside box should work.
    EXPECT_TRUE(intersect(0, 0,  0, -Vector3f::AxisZ()));
    EXPECT_CLOSE(5.f, dist);
    EXPECT_PTS_CLOSE(Point3f(0, 0, -5),      hit.point);
    EXPECT_VECS_CLOSE(Vector3f(0, 0, -1),    hit.normal);
    EXPECT_EQ(Vector3i(0, 2, 6),             hit.indices);
    EXPECT_VECS_CLOSE(Vector3f(.5f, 0, .5f), hit.barycentric);

    // Close misses.
    EXPECT_FALSE(intersect(-20.1f, 0,     15, -Vector3f::AxisZ()));
    EXPECT_FALSE(intersect(0,      10.1f, 15, -Vector3f::AxisZ()));

    // Start past box.
    EXPECT_FALSE(intersect(0, 0, -6, -Vector3f::AxisZ()));

    // Bad ray.
    EXPECT_FALSE(intersect(0, 0, 15, Vector3f::Zero()));
}

TEST_F(IntersectionTest, RaySphereIntersect) {
    float        dist;
    const auto intersect = [&](float x, float y, float z, const Vector3f &dir,
                               float radius){
        return RaySphereIntersect(Ray(Point3f(x, y, z), dir), radius, dist);
    };

    EXPECT_TRUE(intersect(0, 0, 20, -Vector3f::AxisZ(), 10));
    EXPECT_CLOSE(10.f, dist);
    EXPECT_TRUE(intersect(-20, 0, 0, Vector3f::AxisX(), 10));
    EXPECT_CLOSE(10.f, dist);

    // Inside sphere should work.
    EXPECT_TRUE(intersect(0, 0,  0, -Vector3f::AxisZ(), 10));
    EXPECT_CLOSE(10.f, dist);

    // Close misses.
    EXPECT_FALSE(intersect(-10.1f, 0,     20, -Vector3f::AxisZ(), 10));
    EXPECT_FALSE(intersect(0,      10.1f, 20, -Vector3f::AxisZ(), 10));

    // Start past sphere.
    EXPECT_FALSE(intersect(0, 0, -6, -Vector3f::AxisZ(), 2));

    // Bad ray.
    EXPECT_FALSE(intersect(0, 0, 15, Vector3f::Zero(), 4));
}

TEST_F(IntersectionTest, RayCylinderIntersect) {
    float dist;
    const auto intersect = [&](float x, float y, float z, const Vector3f &dir,
                               float radius){
        return RayCylinderIntersect(Ray(Point3f(x, y, z), dir), radius, dist);
    };

    EXPECT_TRUE(intersect(0, 0, 20, -Vector3f::AxisZ(), 10));
    EXPECT_CLOSE(10.f, dist);
    EXPECT_TRUE(intersect(-20, 1000, 0, Vector3f::AxisX(), 10));
    EXPECT_CLOSE(10.f, dist);

    // Inside cylinder should work.
    EXPECT_TRUE(intersect(0, 0,  0, -Vector3f::AxisZ(), 10));
    EXPECT_CLOSE(10.f, dist);

    // Close miss.
    EXPECT_FALSE(intersect(-10.1f, 0,     20, -Vector3f::AxisZ(), 10));

    // Start past cylinder.
    EXPECT_FALSE(intersect(0, 0, -6, -Vector3f::AxisZ(), 2));

    // Close to cylinder axis direction.
    EXPECT_FALSE(intersect(0, 0, 15, Vector3f::AxisY(), 4));

    // Bad ray.
    EXPECT_FALSE(intersect(0, 0, 15, Vector3f::Zero(), 4));
}

TEST_F(IntersectionTest, RayConeIntersect) {
    float dist;
    const Point3f  apex(0, 10, 0);
    const Vector3f axis = -Vector3f::AxisY();
    const Anglef   half_angle = Anglef::FromDegrees(45);
    const auto intersect = [&](float x, float y, float z, const Vector3f &dir){
        return RayConeIntersect(Ray(Point3f(x, y, z), dir),
                                apex, axis, half_angle, dist);
    };

    EXPECT_TRUE(intersect(0, 0, 20, -Vector3f::AxisZ()));
    EXPECT_CLOSE(10.f, dist);
    EXPECT_TRUE(intersect(-20, -1000, 0, Vector3f::AxisX()));

    // Inside cone should work.
    EXPECT_TRUE(intersect(0, 0,  0, -Vector3f::AxisZ()));
    EXPECT_CLOSE(10.f, dist);
    EXPECT_TRUE(intersect(0, 0,  0,  Vector3f::AxisZ()));
    EXPECT_CLOSE(10.f, dist);

    // Hitting only part of cone that does not exist.
    EXPECT_FALSE(intersect(0, 10.1f, 20, -Vector3f::AxisZ()));

    // Hitting part of cone that does not exist first, but real part second.
    EXPECT_TRUE(intersect(10, 20, 0, -Vector3f::AxisY()));
    EXPECT_CLOSE(20.f, dist);

    // Close miss.
    EXPECT_FALSE(intersect(-10.1f, 0,     20, -Vector3f::AxisZ()));

    // Start past cone.
    EXPECT_FALSE(intersect(0, 0, -20, -Vector3f::AxisZ()));

    // Close to cone axis direction.
    EXPECT_FALSE(intersect(0, 0, 15, Vector3f::AxisY()));

    // Bad ray.
    EXPECT_FALSE(intersect(0, 0, 15, Vector3f::Zero()));
}

TEST_F(IntersectionTest, SphereBoundsIntersect) {
    const Bounds b0(Vector3f(10, 20, 40));
    const Bounds b1(Point3f(-4, -3, 6), Point3f(-2, 9, 12));

    float dist;
    EXPECT_TRUE(SphereBoundsIntersect(Point3f(0, 0, 0), 10, b0, dist));
    EXPECT_CLOSE(0.f, dist);
    EXPECT_TRUE(SphereBoundsIntersect(Point3f(10, 0, 0), 10, b0, dist));
    EXPECT_CLOSE(5.f, dist);
    EXPECT_TRUE(SphereBoundsIntersect(Point3f(-10, 0, 0), 10, b0, dist));
    EXPECT_CLOSE(5.f, dist);
    EXPECT_FALSE(SphereBoundsIntersect(Point3f(30, 0, 0), 10, b0, dist));

    EXPECT_TRUE(SphereBoundsIntersect(Point3f(-3, 3, 9), 10, b1, dist));
    EXPECT_CLOSE(0.f, dist);
    EXPECT_TRUE(SphereBoundsIntersect(Point3f(-3, 3, 21), 10, b1, dist));
    EXPECT_CLOSE(9.f, dist);
}
