#include <string>

#include "Math/Types.h"
#include "SG/Node.h"
#include "Tests/SceneTestBase.h"

class NodeBoundsTest : public SceneTestBase {
};

TEST_F(NodeBoundsTest, NoShapes) {
    const std::string input = str1 + str2;
    SG::ScenePtr scene = ReadScene(input);
    EXPECT_NOT_NULL(scene->GetRootNode());
    EXPECT_TRUE(scene->GetRootNode()->GetBounds().IsEmpty());
}

TEST_F(NodeBoundsTest, Box) {
    const std::string input = str1 + "shapes: [Box { size: 2 3 4 }]," + str2;
    SG::ScenePtr scene = ReadScene(input);
    EXPECT_NOT_NULL(scene->GetRootNode());
    const Bounds &bounds = scene->GetRootNode()->GetBounds();
    EXPECT_FALSE(bounds.IsEmpty());
    EXPECT_EQ(Point3f::Zero(),   bounds.GetCenter());
    EXPECT_EQ(Vector3f(2, 3, 4), bounds.GetSize());
}

TEST_F(NodeBoundsTest, Cylinder) {
    const std::string input = str1 +
        "shapes: [Cylinder { height: 6, top_radius: 4, bottom_radius: 3 }]," +
        str2;
    SG::ScenePtr scene = ReadScene(input);
    EXPECT_NOT_NULL(scene->GetRootNode());
    const Bounds &bounds = scene->GetRootNode()->GetBounds();
    EXPECT_FALSE(bounds.IsEmpty());
    EXPECT_EQ(Point3f::Zero(),   bounds.GetCenter());
    EXPECT_EQ(Vector3f(8, 6, 8), bounds.GetSize());
}

TEST_F(NodeBoundsTest, Ellipsoid) {
    const std::string input = str1 +
        "shapes: [Ellipsoid { size: 6 7 8 }]," + str2;
    SG::ScenePtr scene = ReadScene(input);
    EXPECT_NOT_NULL(scene->GetRootNode());
    const Bounds &bounds = scene->GetRootNode()->GetBounds();
    EXPECT_FALSE(bounds.IsEmpty());
    EXPECT_EQ(Point3f::Zero(),   bounds.GetCenter());
    EXPECT_EQ(Vector3f(6, 7, 8), bounds.GetSize());
}

TEST_F(NodeBoundsTest, RegularPolygon) {
    const std::string input = str1 +
        "shapes: [RegularPolygon {"
        " plane_normal: \"kNegativeY\", sides: 11 }]," + str2;
    SG::ScenePtr scene = ReadScene(input);
    EXPECT_NOT_NULL(scene->GetRootNode());
    const Bounds &bounds = scene->GetRootNode()->GetBounds();
    EXPECT_FALSE(bounds.IsEmpty());
    EXPECT_EQ(Point3f::Zero(),     bounds.GetCenter());
    EXPECT_EQ(Vector3f(2, .001f, 2), bounds.GetSize());
}

TEST_F(NodeBoundsTest, Rectangle) {
    const std::string input = str1 +
        "shapes: [Rectangle { plane_normal: \"kNegativeZ\", size: 3 4 }]," +
        str2;
    SG::ScenePtr scene = ReadScene(input);
    EXPECT_NOT_NULL(scene->GetRootNode());
    const Bounds &bounds = scene->GetRootNode()->GetBounds();
    EXPECT_FALSE(bounds.IsEmpty());
    EXPECT_EQ(Point3f::Zero(),      bounds.GetCenter());
    EXPECT_EQ(Vector3f(3, 4, .001), bounds.GetSize());
}

TEST_F(NodeBoundsTest, CombineShapes) {
    const std::string input = str1 +
        "shapes: [Box { size: 2 3 4 }, Box { size: 8 1 6 }]," + str2;
    SG::ScenePtr scene = ReadScene(input);
    EXPECT_NOT_NULL(scene->GetRootNode());
    const Bounds &bounds = scene->GetRootNode()->GetBounds();
    EXPECT_FALSE(bounds.IsEmpty());
    EXPECT_EQ(Point3f::Zero(),   bounds.GetCenter());
    EXPECT_EQ(Vector3f(8, 3, 6), bounds.GetSize());
}

TEST_F(NodeBoundsTest, TransformedRoot) {
    const std::string input = str1 +
        "  scale: 3 4 5,\n"
        "  translation: 100 200 300,\n"
        "  shapes: [Box { size: 2 3 4 }]," + str2;
    SG::ScenePtr scene = ReadScene(input);
    EXPECT_NOT_NULL(scene->GetRootNode());
    const Bounds &bounds = scene->GetRootNode()->GetBounds();
    EXPECT_FALSE(bounds.IsEmpty());
    // The root bounds should be in local coordinates.
    EXPECT_EQ(Point3f::Zero(), bounds.GetCenter());
    EXPECT_EQ(Vector3f(2, 3, 4),    bounds.GetSize());
}

TEST_F(NodeBoundsTest, TransformedChild) {
    const std::string input = str1 +
        "  scale: 2 2 2,\n"
        "  translation: 10 20 30,\n"
        "  children: [\n"
        "    Node \"Child\" {\n"
        "      scale: 3 4 5,\n"
        "      translation: 100 200 300,\n"
        "      shapes: [Box { size: 2 3 4 }],\n"
        "    },\n"
        "  ],\n" + str2;
    SG::ScenePtr scene = ReadScene(input);
    EXPECT_NOT_NULL(scene->GetRootNode());
    const Bounds &bounds = scene->GetRootNode()->GetBounds();
    EXPECT_FALSE(bounds.IsEmpty());
    EXPECT_EQ(Point3f(100, 200, 300), bounds.GetCenter());
    EXPECT_EQ(Vector3f(6, 12, 20),    bounds.GetSize());
}

