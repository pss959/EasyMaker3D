#include <string>

#include "SG/Intersector.h"
#include "SG/Math.h"
#include "SG/Node.h"
#include "SceneTestBase.h"

class IntersectorTest : public SceneTestBase {
  protected:
    // Reads a scene from a string and intersects it with the given ray.
    SG::Hit IntersectScene(const std::string &input, const SG::Ray &ray) {
        SG::ScenePtr scene = ReadScene(input);
        return SG::Intersector::IntersectScene(*scene, ray);
    }
};

TEST_F(IntersectorTest, EmptyScene) {
    const std::string input = "Scene {}\n";
    const SG::Hit hit = IntersectScene(input, SG::Ray(SG::Point3f(0, 20, 0),
                                                      SG::Vector3f(0, 0, -1)));
    EXPECT_TRUE(hit.path.empty());
    EXPECT_NULL(hit.shape);
}

TEST_F(IntersectorTest, Sphere) {
    std::string input = ReadDataFile("Shapes.mvn");
    const SG::Hit hit = IntersectScene(input, SG::Ray(SG::Point3f(0, 20, 0),
                                                      SG::Vector3f(0, 0, -1)));
    EXPECT_FALSE(hit.path.empty());
    EXPECT_NOT_NULL(hit.shape);
}
