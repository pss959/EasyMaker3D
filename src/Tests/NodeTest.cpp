#include <string>

#include "Parser/Registry.h"
#include "SG/Node.h"
#include "SceneTestBase.h"

class NodeTest : public SceneTestBase {};

TEST_F(NodeTest, DefaultNode) {
    SG::NodePtr node = CreateObject<SG::Node>("TestNode");
    EXPECT_NOT_NULL(node);
    EXPECT_EQ("TestNode", node->GetName());
    EXPECT_EQ(0U, node->GetChildCount());
    EXPECT_EQ(0U, node->GetShapes().size());
}

TEST_F(NodeTest, BasicFields) {
    SG::NodePtr node = CreateObject<SG::Node>();
    const Vector3f scale(1, 2, 3);
    const Rotationf rot = Rotationf::FromAxisAndAngle(Vector3f(0, 1, 0),
                                                      Anglef::FromDegrees(30));
    const Vector3f trans(10, 20, 30);
    node->SetScale(scale);
    node->SetRotation(rot);
    node->SetTranslation(trans);
    EXPECT_EQ(0U, node->GetChildCount());
    EXPECT_EQ(0U, node->GetShapes().size());
    EXPECT_EQ(scale, node->GetScale());
    EXPECT_EQ(rot,   node->GetRotation());
    EXPECT_EQ(trans, node->GetTranslation());
}

TEST_F(NodeTest, Enable) {
    SG::NodePtr node = CreateObject<SG::Node>();
    EXPECT_TRUE(node->IsEnabled(SG::Node::Flag::kTraversal));
    EXPECT_TRUE(node->IsEnabled(SG::Node::Flag::kRender));
    EXPECT_TRUE(node->IsEnabled(SG::Node::Flag::kIntersect));
    EXPECT_TRUE(node->IsEnabled(SG::Node::Flag::kIntersectAll));
    node->SetEnabled(SG::Node::Flag::kTraversal, false);
    EXPECT_FALSE(node->IsEnabled(SG::Node::Flag::kTraversal));
    EXPECT_TRUE(node->IsEnabled(SG::Node::Flag::kRender));
    EXPECT_TRUE(node->IsEnabled(SG::Node::Flag::kIntersect));
    EXPECT_TRUE(node->IsEnabled(SG::Node::Flag::kIntersectAll));
    node->SetEnabled(SG::Node::Flag::kIntersectAll, false);
    EXPECT_FALSE(node->IsEnabled(SG::Node::Flag::kTraversal));
    EXPECT_TRUE(node->IsEnabled(SG::Node::Flag::kRender));
    EXPECT_TRUE(node->IsEnabled(SG::Node::Flag::kIntersect));
    EXPECT_FALSE(node->IsEnabled(SG::Node::Flag::kIntersectAll));
    node->SetEnabled(SG::Node::Flag::kTraversal, true);
    EXPECT_TRUE(node->IsEnabled(SG::Node::Flag::kTraversal));
    EXPECT_TRUE(node->IsEnabled(SG::Node::Flag::kRender));
    EXPECT_TRUE(node->IsEnabled(SG::Node::Flag::kIntersect));
    EXPECT_FALSE(node->IsEnabled(SG::Node::Flag::kIntersectAll));
    node->SetEnabled(SG::Node::Flag::kIntersectAll, true);
    EXPECT_TRUE(node->IsEnabled(SG::Node::Flag::kTraversal));
    EXPECT_TRUE(node->IsEnabled(SG::Node::Flag::kRender));
    EXPECT_TRUE(node->IsEnabled(SG::Node::Flag::kIntersect));
    EXPECT_TRUE(node->IsEnabled(SG::Node::Flag::kIntersectAll));
}
