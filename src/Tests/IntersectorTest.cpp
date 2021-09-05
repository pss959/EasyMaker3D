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
    EXPECT_PRED2(PointsClose,  Point3f(-100, 0, 5), hit.point);
    EXPECT_PRED2(VectorsClose, Vector3f(0, 0, 1),   hit.normal);

    // Intersect from bottom.
    hit = IntersectScene(input, Ray(Point3f(-100, -10, 0),
                                        Vector3f(0, 1, 0)));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    EXPECT_NOT_NULL(hit.shape);
    EXPECT_NEAR(5.f, hit.distance, kClose);  // Sphere has radius 5.
    EXPECT_PRED2(PointsClose,  Point3f(-100, -5, 0), hit.point);
    EXPECT_PRED2(VectorsClose, Vector3f(0, -1, 0),   hit.normal);
}

TEST_F(IntersectorTest, Cone) {
    std::string input = ReadDataFile("Shapes.mvn");

    // Intersect from front. Cone is at (100,0,0).
    SG::Hit hit = IntersectScene(input, Ray(Point3f(100, 0, 20),
                                            Vector3f(0, 0, -1)));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    EXPECT_NOT_NULL(hit.shape);
    EXPECT_NEAR(5.f, hit.distance, kClose);
    EXPECT_PRED2(PointsClose,  Point3f(100, 0, -20), hit.point);
    EXPECT_PRED2(VectorsClose, ion::math::Normalized(Vector3f(0, 1, 1)),
                 hit.normal);
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
