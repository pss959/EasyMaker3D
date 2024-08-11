//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Math/Types.h"
#include "SG/Node.h"
#include "SG/Torus.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class NodeBoundsTest : public SceneTestBase {};

TEST_F(NodeBoundsTest, NoShapes) {
    SG::ScenePtr scene = BuildAndReadScene("");
    EXPECT_NOT_NULL(scene->GetRootNode());
    EXPECT_TRUE(scene->GetRootNode()->GetBounds().IsEmpty());
}

TEST_F(NodeBoundsTest, Box) {
    SG::ScenePtr scene = BuildAndReadScene("shapes: [Box { size: 2 3 4 }],");
    EXPECT_NOT_NULL(scene->GetRootNode());
    const Bounds &bounds = scene->GetRootNode()->GetBounds();
    EXPECT_FALSE(bounds.IsEmpty());
    EXPECT_EQ(Point3f::Zero(),   bounds.GetCenter());
    EXPECT_EQ(Vector3f(2, 3, 4), bounds.GetSize());
}

TEST_F(NodeBoundsTest, Cylinder) {
    SG::ScenePtr scene = BuildAndReadScene(
        "shapes: [Cylinder { height: 6, top_radius: 4, bottom_radius: 3 }],");
    EXPECT_NOT_NULL(scene->GetRootNode());
    const Bounds &bounds = scene->GetRootNode()->GetBounds();
    EXPECT_FALSE(bounds.IsEmpty());
    EXPECT_EQ(Point3f::Zero(),   bounds.GetCenter());
    EXPECT_EQ(Vector3f(8, 6, 8), bounds.GetSize());
}

TEST_F(NodeBoundsTest, Ellipsoid) {
    SG::ScenePtr scene =
        BuildAndReadScene("shapes: [Ellipsoid { size: 6 7 8 }],");
    EXPECT_NOT_NULL(scene->GetRootNode());
    const Bounds &bounds = scene->GetRootNode()->GetBounds();
    EXPECT_FALSE(bounds.IsEmpty());
    EXPECT_EQ(Point3f::Zero(),   bounds.GetCenter());
    EXPECT_EQ(Vector3f(6, 7, 8), bounds.GetSize());
}

TEST_F(NodeBoundsTest, RegularPolygon) {
    SG::ScenePtr scene = BuildAndReadScene(
        "shapes: [RegularPolygon { plane_normal: \"kNegativeY\","
        " sides: 11 }],");
    EXPECT_NOT_NULL(scene->GetRootNode());
    const Bounds &bounds = scene->GetRootNode()->GetBounds();
    EXPECT_FALSE(bounds.IsEmpty());
    EXPECT_EQ(Point3f::Zero(),     bounds.GetCenter());
    EXPECT_EQ(Vector3f(2, .001f, 2), bounds.GetSize());
}

TEST_F(NodeBoundsTest, Rectangle) {
    SG::ScenePtr scene = BuildAndReadScene(
        "shapes: [Rectangle { plane_normal: \"kNegativeZ\", size: 3 4 }],");
    EXPECT_NOT_NULL(scene->GetRootNode());
    const Bounds &bounds = scene->GetRootNode()->GetBounds();
    EXPECT_FALSE(bounds.IsEmpty());
    EXPECT_EQ(Point3f::Zero(),      bounds.GetCenter());
    EXPECT_EQ(Vector3f(3, 4, .001), bounds.GetSize());
}

TEST_F(NodeBoundsTest, CombineShapes) {
    SG::ScenePtr scene = BuildAndReadScene(
        "shapes: [Box { size: 2 3 4 }, Box { size: 8 1 6 }],");
    EXPECT_NOT_NULL(scene->GetRootNode());
    const Bounds &bounds = scene->GetRootNode()->GetBounds();
    EXPECT_FALSE(bounds.IsEmpty());
    EXPECT_EQ(Point3f::Zero(),   bounds.GetCenter());
    EXPECT_EQ(Vector3f(8, 3, 6), bounds.GetSize());
}

TEST_F(NodeBoundsTest, TransformedRoot) {
    const Str contents = R"(
  scale: 3 4 5,
  translation: 100 200 300,
  shapes: [Box { size: 2 3 4 }],
)";
    SG::ScenePtr scene = BuildAndReadScene(contents);
    EXPECT_NOT_NULL(scene->GetRootNode());
    const Bounds &bounds = scene->GetRootNode()->GetBounds();
    EXPECT_FALSE(bounds.IsEmpty());
    // The root bounds should be in local coordinates.
    EXPECT_EQ(Point3f::Zero(), bounds.GetCenter());
    EXPECT_EQ(Vector3f(2, 3, 4),    bounds.GetSize());

    const Bounds &sbounds = scene->GetRootNode()->GetScaledBounds();
    EXPECT_FALSE(bounds.IsEmpty());
    EXPECT_EQ(Point3f::Zero(),     sbounds.GetCenter());
    EXPECT_EQ(Vector3f(6, 12, 20), sbounds.GetSize());
}

TEST_F(NodeBoundsTest, TransformedChild) {
    const Str contents = R"(
  scale: 2 2 2,
  translation: 10 20 30,
  children: [
    Node "Child" {
      scale: 3 4 5,
      translation: 100 200 300,
      shapes: [Torus { outer_radius: 4, inner_radius: 1 }],
    },
  ],
)";
    SG::ScenePtr scene = BuildAndReadScene(contents);
    EXPECT_NOT_NULL(scene->GetRootNode());
    Bounds bounds = scene->GetRootNode()->GetBounds();
    EXPECT_FALSE(bounds.IsEmpty());
    EXPECT_EQ(Point3f(100, 200, 300), bounds.GetCenter());
    EXPECT_EQ(Vector3f(24, 8, 40),    bounds.GetSize());

    // Test notification from the Box shape to update the bounds.
    EXPECT_EQ(1U, scene->GetRootNode()->GetChildren().size());
    auto node = scene->GetRootNode()->GetChildren()[0];
    EXPECT_EQ("Child", node->GetName());
    EXPECT_EQ(1U, node->GetShapes().size());
    auto torus = std::dynamic_pointer_cast<SG::Torus>(node->GetShapes()[0]);
    EXPECT_NOT_NULL(torus);
    torus->SetOuterRadius(10);
    bounds = scene->GetRootNode()->GetBounds();
    EXPECT_EQ(Vector3f(60, 8, 100), bounds.GetSize());

    // Disabling notification in the shape should NOT affect the bounds.
    torus->SetNotifyEnabled(false);
    torus->SetOuterRadius(20);
    bounds = scene->GetRootNode()->GetBounds();
    EXPECT_EQ(Vector3f(60, 8, 100), bounds.GetSize());

    // Same for the node.
    node->SetNotifyEnabled(false);
    node->SetUniformScale(100);
    bounds = scene->GetRootNode()->GetBounds();
    EXPECT_EQ(Vector3f(60, 8, 100), bounds.GetSize());
}
