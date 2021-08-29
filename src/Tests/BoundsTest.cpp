#include <string>

#include "SG/Math.h"
#include "SG/Node.h"
#include "SceneTestBase.h"

class BoundsTest : public SceneTestBase {
};

TEST_F(BoundsTest, NoShapes) {
    const std::string input =
        "Scene { render_passes: [LightingPass { root: Node {} }]}}\n";
    SG::ScenePtr scene = ReadScene(input);
    EXPECT_NOT_NULL(scene->GetRootNode());
    EXPECT_TRUE(scene->GetRootNode()->GetBounds().IsEmpty());
}

TEST_F(BoundsTest, Box) {
    const std::string input =
        "Scene { render_passes: [LightingPass { root: Node {\n"
        "  shapes: [Box { size: 2 3 4 }],\n"
        "} }]}}\n";
    SG::ScenePtr scene = ReadScene(input);
    EXPECT_NOT_NULL(scene->GetRootNode());
    const SG::Bounds &bounds = scene->GetRootNode()->GetBounds();
    EXPECT_FALSE(bounds.IsEmpty());
    EXPECT_EQ(SG::Point3f::Zero(),   bounds.GetCenter());
    EXPECT_EQ(SG::Vector3f(2, 3, 4), bounds.GetSize());
}

TEST_F(BoundsTest, Cylinder) {
    const std::string input =
        "Scene { render_passes: [LightingPass { root: Node {\n"
        "  shapes: [Cylinder { height: 6, top_radius: 4, bottom_radius: 3 }],\n"
        "} }]}}\n";
    SG::ScenePtr scene = ReadScene(input);
    EXPECT_NOT_NULL(scene->GetRootNode());
    const SG::Bounds &bounds = scene->GetRootNode()->GetBounds();
    EXPECT_FALSE(bounds.IsEmpty());
    EXPECT_EQ(SG::Point3f::Zero(),   bounds.GetCenter());
    EXPECT_EQ(SG::Vector3f(8, 6, 8), bounds.GetSize());
}

TEST_F(BoundsTest, Ellipsoid) {
    const std::string input =
        "Scene { render_passes: [LightingPass { root: Node {\n"
        "  shapes: [Ellipsoid { size: 6 7 8 }],\n"
        "} }]}}\n";
    SG::ScenePtr scene = ReadScene(input);
    EXPECT_NOT_NULL(scene->GetRootNode());
    const SG::Bounds &bounds = scene->GetRootNode()->GetBounds();
    EXPECT_FALSE(bounds.IsEmpty());
    EXPECT_EQ(SG::Point3f::Zero(),   bounds.GetCenter());
    EXPECT_EQ(SG::Vector3f(6, 7, 8), bounds.GetSize());
}

TEST_F(BoundsTest, Polygon) {
    const std::string input =
        "Scene { render_passes: [LightingPass { root: Node {\n"
        "  shapes: [Polygon { plane_normal: \"kNegativeY\", sides: 11 }],\n"
        "} }]}}\n";
    SG::ScenePtr scene = ReadScene(input);
    EXPECT_NOT_NULL(scene->GetRootNode());
    const SG::Bounds &bounds = scene->GetRootNode()->GetBounds();
    EXPECT_FALSE(bounds.IsEmpty());
    EXPECT_EQ(SG::Point3f::Zero(),     bounds.GetCenter());
    EXPECT_EQ(SG::Vector3f(2, .01, 2), bounds.GetSize());
}

TEST_F(BoundsTest, Rectangle) {
    const std::string input =
        "Scene { render_passes: [LightingPass { root: Node {\n"
        "  shapes: [Rectangle { plane_normal: \"kNegativeZ\", size: 3 4 }],\n"
        "} }]}}\n";
    SG::ScenePtr scene = ReadScene(input);
    EXPECT_NOT_NULL(scene->GetRootNode());
    const SG::Bounds &bounds = scene->GetRootNode()->GetBounds();
    EXPECT_FALSE(bounds.IsEmpty());
    EXPECT_EQ(SG::Point3f::Zero(),      bounds.GetCenter());
    EXPECT_EQ(SG::Vector3f(3, 4, .001), bounds.GetSize());
}

TEST_F(BoundsTest, CombineShapes) {
    const std::string input =
        "Scene { render_passes: [LightingPass { root: Node {\n"
        "  shapes: [Box { size: 2 3 4 }, Box { size: 8 1 6 }],\n"
        "} }]}}\n";
    SG::ScenePtr scene = ReadScene(input);
    EXPECT_NOT_NULL(scene->GetRootNode());
    const SG::Bounds &bounds = scene->GetRootNode()->GetBounds();
    EXPECT_FALSE(bounds.IsEmpty());
    EXPECT_EQ(SG::Point3f::Zero(),   bounds.GetCenter());
    EXPECT_EQ(SG::Vector3f(8, 3, 6), bounds.GetSize());
}

