#include <string>

#include "Parser/Registry.h"
#include "SG/Box.h"
#include "SG/Node.h"
#include "SG/UnscopedNode.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Util/Assert.h"

/// \ingroup Tests
class NodeTest : public SceneTestBase {};

TEST_F(NodeTest, DefaultNode) {
    SG::NodePtr node = CreateObject<SG::Node>("TestNode");
    EXPECT_NOT_NULL(node);
    EXPECT_EQ("TestNode", node->GetName());
    EXPECT_EQ("", node->GetPassName());
    EXPECT_EQ(0U, node->GetChildCount());
    EXPECT_EQ(0U, node->GetShapes().size());
}

TEST_F(NodeTest, Transform) {
    SG::NodePtr node = CreateObject<SG::Node>();

    // Default transform.
    EXPECT_EQ(Vector3f(1, 1, 1),     node->GetScale());
    EXPECT_EQ(Rotationf::Identity(), node->GetRotation());
    EXPECT_EQ(Vector3f(0, 0, 0),     node->GetTranslation());

    // Set values.
    const Vector3f scale(1, 2, 3);
    const Rotationf rot = BuildRotation(0, 1, 0, 30);
    const Vector3f trans(10, 20, 30);
    node->SetScale(scale);
    node->SetRotation(rot);
    node->SetTranslation(trans);
    EXPECT_EQ(0U, node->GetChildCount());
    EXPECT_EQ(0U, node->GetShapes().size());
    EXPECT_EQ(scale, node->GetScale());
    EXPECT_EQ(rot,   node->GetRotation());
    EXPECT_EQ(trans, node->GetTranslation());

    // Other ways to set values.
    node->SetUniformScale(12);
    EXPECT_EQ(Vector3f(12, 12, 12), node->GetScale());
    node->TranslateTo(Point3f(4, -2, 1));
    EXPECT_EQ(Vector3f(4, -2, 1), node->GetTranslation());

    // Copy.
    SG::NodePtr copy = CreateObject<SG::Node>();
    copy->CopyTransformsFrom(*node);
    EXPECT_EQ(node->GetScale(),       copy->GetScale());
    EXPECT_EQ(node->GetRotation(),    copy->GetRotation());
    EXPECT_EQ(node->GetTranslation(), copy->GetTranslation());

    // Reset.
    node->ResetTransform();
    EXPECT_EQ(Vector3f(1, 1, 1),     node->GetScale());
    EXPECT_EQ(Rotationf::Identity(), node->GetRotation());
    EXPECT_EQ(Vector3f(0, 0, 0),     node->GetTranslation());
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

    node->SetEnabled(false);
    EXPECT_FALSE(node->IsFlagEnabled(SG::Node::Flag::kTraversal));
    EXPECT_TRUE(node->IsFlagEnabled(SG::Node::Flag::kRender));
    EXPECT_TRUE(node->IsFlagEnabled(SG::Node::Flag::kIntersect));
    EXPECT_TRUE(node->IsFlagEnabled(SG::Node::Flag::kIntersectAll));
    node->SetEnabled(true);
    EXPECT_TRUE(node->IsFlagEnabled(SG::Node::Flag::kTraversal));
    EXPECT_TRUE(node->IsFlagEnabled(SG::Node::Flag::kRender));
    EXPECT_TRUE(node->IsFlagEnabled(SG::Node::Flag::kIntersect));
    EXPECT_TRUE(node->IsFlagEnabled(SG::Node::Flag::kIntersectAll));
}

TEST_F(NodeTest, Shapes) {
    SG::NodePtr node = CreateObject<SG::Node>();
    EXPECT_EQ(0U, node->GetShapes().size());

    // Call SetUpIon() first so that shape addition and clearing also tests the
    // Ion shape handling.
    SetUpIonForNode(*node);

    auto box0 = CreateObject<SG::Box>();
    auto box1 = CreateObject<SG::Box>();
    node->AddShape(box0);
    EXPECT_EQ(1U,   node->GetShapes().size());
    EXPECT_EQ(box0, node->GetShapes()[0]);
    node->AddShape(box1);
    EXPECT_EQ(2U,   node->GetShapes().size());
    EXPECT_EQ(box0, node->GetShapes()[0]);
    EXPECT_EQ(box1, node->GetShapes()[1]);

    node->ClearShapes();
    EXPECT_EQ(0U, node->GetShapes().size());
}

TEST_F(NodeTest, Children) {
    SG::NodePtr parent = CreateObject<SG::Node>("Parent");
    SG::NodePtr a      = CreateObject<SG::Node>("A");
    SG::NodePtr b      = CreateObject<SG::Node>("B");
    SG::NodePtr c      = CreateObject<SG::Node>("C");
    SG::NodePtr d      = CreateObject<SG::Node>("D");

    // Call SetUpIon() first so that this also tests Ion Node handling.
    SetUpIonForNode(*parent);

    auto test_it = [&](const Str &expected) {
        EXPECT_EQ(expected.size(), parent->GetChildCount());
        Str actual;
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

    parent->InsertChild(3, d);
    test_it("ACBD");

    EXPECT_EQ(0,  parent->GetChildIndex(a));
    EXPECT_EQ(1,  parent->GetChildIndex(c));
    EXPECT_EQ(2,  parent->GetChildIndex(b));
    EXPECT_EQ(3,  parent->GetChildIndex(d));
    EXPECT_EQ(-1, a->GetChildIndex(b));

    parent->RemoveChild(b);
    test_it("ACD");

    TEST_THROW(parent->RemoveChild(b), AssertException, "index >= 0");

    parent->ClearChildren();
    test_it("");
}

TEST_F(NodeTest, ExtraChildren) {
    SG::NodePtr parent = CreateObject<SG::Node>("Parent");
    SG::NodePtr a      = CreateObject<SG::Node>("A");
    SG::NodePtr b      = CreateObject<SG::Node>("B");

    EXPECT_EQ(0U, parent->GetExtraChildren().size());
    parent->AddExtraChild(a);
    EXPECT_EQ(1U, parent->GetExtraChildren().size());
    EXPECT_EQ(a,  parent->GetExtraChildren()[0]);
    parent->AddExtraChild(b);
    EXPECT_EQ(2U, parent->GetExtraChildren().size());
    EXPECT_EQ(a,  parent->GetExtraChildren()[0]);
    EXPECT_EQ(b,  parent->GetExtraChildren()[1]);

    parent->ClearExtraChildren();
    EXPECT_EQ(0U, parent->GetExtraChildren().size());
}

TEST_F(NodeTest, UnscopedNode) {
    SG::NodePtr sn = CreateObject<SG::Node>();
    SG::NodePtr un = CreateObject<SG::UnscopedNode>();
    EXPECT_TRUE(sn->IsScoped());
    EXPECT_FALSE(un->IsScoped());
}
