#include <string>

#include "Parser/Registry.h"
#include "SG/Node.h"
#include "Tests/SceneTestBase.h"

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
    EXPECT_TRUE(node->IsFlagEnabled(SG::Node::Flag::kTraversal));
    EXPECT_TRUE(node->IsFlagEnabled(SG::Node::Flag::kRender));
    EXPECT_TRUE(node->IsFlagEnabled(SG::Node::Flag::kIntersect));
    EXPECT_TRUE(node->IsFlagEnabled(SG::Node::Flag::kIntersectAll));
    node->SetFlagEnabled(SG::Node::Flag::kTraversal, false);
    EXPECT_FALSE(node->IsFlagEnabled(SG::Node::Flag::kTraversal));
    EXPECT_TRUE(node->IsFlagEnabled(SG::Node::Flag::kRender));
    EXPECT_TRUE(node->IsFlagEnabled(SG::Node::Flag::kIntersect));
    EXPECT_TRUE(node->IsFlagEnabled(SG::Node::Flag::kIntersectAll));
    node->SetFlagEnabled(SG::Node::Flag::kIntersectAll, false);
    EXPECT_FALSE(node->IsFlagEnabled(SG::Node::Flag::kTraversal));
    EXPECT_TRUE(node->IsFlagEnabled(SG::Node::Flag::kRender));
    EXPECT_TRUE(node->IsFlagEnabled(SG::Node::Flag::kIntersect));
    EXPECT_FALSE(node->IsFlagEnabled(SG::Node::Flag::kIntersectAll));
    node->SetFlagEnabled(SG::Node::Flag::kTraversal, true);
    EXPECT_TRUE(node->IsFlagEnabled(SG::Node::Flag::kTraversal));
    EXPECT_TRUE(node->IsFlagEnabled(SG::Node::Flag::kRender));
    EXPECT_TRUE(node->IsFlagEnabled(SG::Node::Flag::kIntersect));
    EXPECT_FALSE(node->IsFlagEnabled(SG::Node::Flag::kIntersectAll));
    node->SetFlagEnabled(SG::Node::Flag::kIntersectAll, true);
    EXPECT_TRUE(node->IsFlagEnabled(SG::Node::Flag::kTraversal));
    EXPECT_TRUE(node->IsFlagEnabled(SG::Node::Flag::kRender));
    EXPECT_TRUE(node->IsFlagEnabled(SG::Node::Flag::kIntersect));
    EXPECT_TRUE(node->IsFlagEnabled(SG::Node::Flag::kIntersectAll));
}

TEST_F(NodeTest, Children) {
    SG::NodePtr parent = CreateObject<SG::Node>("Parent");
    SG::NodePtr a      = CreateObject<SG::Node>("A");
    SG::NodePtr b      = CreateObject<SG::Node>("B");
    SG::NodePtr c      = CreateObject<SG::Node>("C");

    auto test_it = [&](const std::string &expected) {
        EXPECT_EQ(expected.size(), parent->GetChildCount());
        std::string actual;
        for (size_t i = 0; i < parent->GetChildCount(); ++i)
            actual += parent->GetChild(i)->GetName();
        EXPECT_EQ(expected, actual);
    };

    test_it("");

    parent->AddChild(a);
    test_it("A");

    parent->AddChild(b);
    test_it("AB");

    parent->AddChild(c);
    test_it("ABC");

    parent->RemoveChild(1);
    test_it("AC");

    parent->ReplaceChild(1, b);
    test_it("AB");

    parent->InsertChild(1, c);
    test_it("ACB");

    parent->ClearChildren();
    test_it("");
}
