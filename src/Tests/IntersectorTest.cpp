#include <string>

#include "Math/Types.h"
#include "SG/Intersector.h"
#include "SG/Node.h"
#include "SceneTestBase.h"

class IntersectorTest : public SceneTestBase {
  protected:
    // Reads a scene from a string and intersects it with the given ray.
    SG::Hit IntersectScene(const std::string &input, const Ray &ray) {
        SG::ScenePtr scene = ReadScene(input);
        return SG::Intersector::IntersectScene(*scene, ray);
    }
};

TEST_F(IntersectorTest, EmptyScene) {
    const std::string input = "Scene {}\n";
    const SG::Hit hit = IntersectScene(input, Ray(Point3f(0, 20, 0),
                                                  Vector3f(0, 0, -1)));
    EXPECT_FALSE(hit.IsValid());
    EXPECT_TRUE(hit.path.empty());
    EXPECT_NULL(hit.shape);
}

TEST_F(IntersectorTest, Sphere) {
    std::string input = ReadDataFile("Shapes.mvn");

    // Intersect from front. Sphere is at (-100,0,0).
    SG::Hit hit = IntersectScene(input, Ray(Point3f(-100, 0, 20),
                                            Vector3f(0, 0, -1)));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    EXPECT_NOT_NULL(hit.shape);
    EXPECT_NEAR(15.f, hit.distance, kClose);  // Sphere has radius 5.
    EXPECT_PTS_CLOSE(Point3f(0, 0, 5),    hit.point);
    EXPECT_PTS_CLOSE(Point3f(-100, 0, 5), hit.GetWorldPoint());
    EXPECT_VECS_CLOSE(Vector3f(0, 0, 1),  hit.normal);

    // Intersect from bottom.
    hit = IntersectScene(input, Ray(Point3f(-100, -10, 0), Vector3f(0, 1, 0)));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    EXPECT_NOT_NULL(hit.shape);
    EXPECT_NEAR(5.f, hit.distance, kClose);  // Sphere has radius 5.
    EXPECT_PTS_CLOSE(Point3f(0, -5, 0),    hit.point);
    EXPECT_PTS_CLOSE(Point3f(-100, -5, 0), hit.GetWorldPoint());
    EXPECT_VECS_CLOSE(Vector3f(0, -1, 0),  hit.normal);
}

TEST_F(IntersectorTest, Cone) {
    std::string input = ReadDataFile("Shapes.mvn");

    // Intersect from front. Cone is at (100,0,0).
    SG::Hit hit = IntersectScene(input, Ray(Point3f(100, 0, 20),
                                            Vector3f(0, 0, -1)));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    EXPECT_NOT_NULL(hit.shape);
    EXPECT_EQ("Cone", hit.shape->GetName());
    EXPECT_NEAR(5.f, hit.distance, kClose);
    EXPECT_PTS_CLOSE(Point3f(0,   0, 15), hit.point);
    EXPECT_PTS_CLOSE(Point3f(100, 0, 15), hit.GetWorldPoint());
    EXPECT_VECS_CLOSE(ion::math::Normalized(Vector3f(0, 1, 1)), hit.normal);
}

TEST_F(IntersectorTest, Torus) {
    std::string input = ReadDataFile("Shapes.mvn");

    // Intersect from front. Torus is at origin with outer radius 1.2 and inner
    // radius .2.
    SG::Hit hit = IntersectScene(input, Ray(Point3f(0, 0, 20),
                                            Vector3f(0, 0, -1)));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    EXPECT_NOT_NULL(hit.shape);
    EXPECT_EQ("Torus", hit.shape->GetName());
    EXPECT_NEAR(18.8f, hit.distance, kClose);
    EXPECT_PTS_CLOSE(Point3f(0, 0, 1.2f), hit.point);
}

TEST_F(IntersectorTest, Rectangles) {
    std::string input = ReadDataFile("Rectangles.mvn");

    // The scene is translated 10 units in X.
    // Intersect rays from the center to all 5 rectangles.

    const Point3f center(10, 0, 0);

    SG::Hit hit = IntersectScene(input, Ray(center, -Vector3f::AxisZ()));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    EXPECT_NOT_NULL(hit.shape);
    EXPECT_EQ("Back", hit.path.back()->GetName());

    hit = IntersectScene(input, Ray(center, Vector3f::AxisX()));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    EXPECT_NOT_NULL(hit.shape);
    EXPECT_EQ("Right", hit.path.back()->GetName());

    hit = IntersectScene(input, Ray(center, -Vector3f::AxisX()));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    EXPECT_NOT_NULL(hit.shape);
    EXPECT_EQ("Left", hit.path.back()->GetName());

    hit = IntersectScene(input, Ray(center, Vector3f::AxisY()));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    EXPECT_NOT_NULL(hit.shape);
    EXPECT_EQ("Top", hit.path.back()->GetName());

    hit = IntersectScene(input, Ray(center, -Vector3f::AxisY()));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    EXPECT_NOT_NULL(hit.shape);
    EXPECT_EQ("Bottom", hit.path.back()->GetName());
}

TEST_F(IntersectorTest, HiddenParent) {
    std::string input = ReadDataFile("Shapes.mvn");

    // The parent Ellipsoid is large and encompasses the two child Ellipsoids.
    // The parent should not be intersected, and its translation should not be
    // applied to the children - if it were, the rays would miss them.

    // Child sphere on the left.
    SG::Hit hit = IntersectScene(input, Ray(Point3f(-2, 0, 0),
                                            Vector3f(0, 0, -1)));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    EXPECT_EQ("Child0", hit.path.back()->GetName());

    // Child sphere on the right.
    hit = IntersectScene(input, Ray(Point3f(2, 0, 0), Vector3f(0, 0, -1)));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    EXPECT_EQ("Child1", hit.path.back()->GetName());
}

TEST_F(IntersectorTest, TranslatedShapes) {
    // Tests intersection with a shape with translation field set.

    std::string input = ReadDataFile("Shapes.mvn");

    // Intersect from front:
    //   Box      is at (304,0,0).
    //   Cylinder is at (324,0,0).
    //   Sphere   is at (344,0,0).

    SG::Hit hit;

    hit = IntersectScene(input, Ray(Point3f(304, 0, 20), Vector3f(0, 0, -1)));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    EXPECT_NOT_NULL(hit.shape);
    EXPECT_EQ("TranslatedBox", hit.shape->GetName());
    EXPECT_NEAR(15.f, hit.distance, kClose);  // Box has size 10.
    EXPECT_PTS_CLOSE(Point3f(4, 0, 5),   hit.point);
    EXPECT_PTS_CLOSE(Point3f(304, 0, 5), hit.GetWorldPoint());
    EXPECT_VECS_CLOSE(Vector3f(0, 0, 1), hit.normal);

    hit = IntersectScene(input, Ray(Point3f(324, 0, 20), Vector3f(0, 0, -1)));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    EXPECT_NOT_NULL(hit.shape);
    EXPECT_EQ("TranslatedCylinder", hit.shape->GetName());
    EXPECT_NEAR(15.f, hit.distance, kClose);  // Cylinder has radius 5.
    EXPECT_PTS_CLOSE(Point3f(24, 0, 5),  hit.point);
    EXPECT_PTS_CLOSE(Point3f(324, 0, 5), hit.GetWorldPoint());
    EXPECT_VECS_CLOSE(Vector3f(0, 0, 1), hit.normal);

    hit = IntersectScene(input, Ray(Point3f(344, 0, 20), Vector3f(0, 0, -1)));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    EXPECT_NOT_NULL(hit.shape);
    EXPECT_EQ("TranslatedSphere", hit.shape->GetName());
    EXPECT_NEAR(15.f, hit.distance, kClose);  // Sphere has radius 5.
    EXPECT_PTS_CLOSE(Point3f(44, 0, 5),  hit.point);
    EXPECT_PTS_CLOSE(Point3f(344, 0, 5), hit.GetWorldPoint());
    EXPECT_VECS_CLOSE(Vector3f(0, 0, 1), hit.normal);
}
