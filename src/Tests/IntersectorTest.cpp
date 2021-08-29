#include <string>

#include <ion/math/vectorutils.h>

#include "SG/Intersector.h"
#include "SG/Math.h"
#include "SG/Node.h"
#include "SceneTestBase.h"

class IntersectorTest : public SceneTestBase {
  protected:
    // Close enough.
    static constexpr float kClose = 1e-4f;

    // Reads a scene from a string and intersects it with the given ray.
    SG::Hit IntersectScene(const std::string &input, const SG::Ray &ray) {
        SG::ScenePtr scene = ReadScene(input);
        return SG::Intersector::IntersectScene(*scene, ray);
    }

    // Tests vectors or points for near equality.
    static bool AlmostEqual(const SG::Point3f &p0, const SG::Point3f &p1) {
        return ion::math::AlmostEqual(p0, p1, kClose);
    }
    static bool AlmostEqual(const SG::Vector3f &v0, const SG::Vector3f &v1) {
        return ion::math::AlmostEqual(v0, v1, kClose);
    }
};

TEST_F(IntersectorTest, EmptyScene) {
    const std::string input = "Scene {}\n";
    const SG::Hit hit = IntersectScene(input, SG::Ray(SG::Point3f(0, 20, 0),
                                                      SG::Vector3f(0, 0, -1)));
    EXPECT_FALSE(hit.IsValid());
    EXPECT_TRUE(hit.path.empty());
    EXPECT_NULL(hit.shape);
}

TEST_F(IntersectorTest, Sphere) {
    std::string input = ReadDataFile("Shapes.mvn");

    // Intersect from front.
    SG::Hit hit = IntersectScene(input, SG::Ray(SG::Point3f(0, 0, 20),
                                                SG::Vector3f(0, 0, -1)));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    EXPECT_NOT_NULL(hit.shape);
    EXPECT_NEAR(15.f, hit.distance, kClose);  // Sphere has radius 5.
    EXPECT_TRUE(AlmostEqual(SG::Point3f(0, 0, 5),  hit.point));
    EXPECT_TRUE(AlmostEqual(SG::Vector3f(0, 0, 1), hit.normal));

    // Intersect from bottom.
    hit = IntersectScene(input, SG::Ray(SG::Point3f(0, -10, 0),
                                        SG::Vector3f(0, 1, 0)));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    EXPECT_NOT_NULL(hit.shape);
    EXPECT_NEAR(5.f, hit.distance, kClose);  // Sphere has radius 5.
    EXPECT_TRUE(AlmostEqual(SG::Point3f(0, -5, 0),  hit.point));
    EXPECT_TRUE(AlmostEqual(SG::Vector3f(0, -1, 0), hit.normal));
}
