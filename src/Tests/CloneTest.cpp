#include <string>

#include "Parser/Registry.h"
#include "SG/Node.h"
#include "SG/Shape.h"
#include "SceneTestBase.h"
#include "Util/General.h"

class CloneTest : public SceneTestBase {};

TEST_F(CloneTest, DefaultNode) {
    SG::NodePtr node =
        Parser::Registry::CreateObject<SG::Node>("Node", "TestNode");
    SG::NodePtr clone = Util::CastToDerived<SG::Node>(node->Clone(true));
    EXPECT_NOT_NULL(clone);
    EXPECT_EQ("TestNode", clone->GetName());
    EXPECT_EQ(0U, clone->GetChildCount());
    EXPECT_EQ(0U, clone->GetShapes().size());
}

TEST_F(CloneTest, BasicFields) {
    SG::NodePtr node = Parser::Registry::CreateObject<SG::Node>("Node");
    node->SetScale(Vector3f(1, 2, 3));
    node->SetTranslation(Vector3f(10, 20, 30));
    SG::NodePtr clone = Util::CastToDerived<SG::Node>(node->Clone(true));
    EXPECT_NOT_NULL(clone);
    EXPECT_EQ(0U, clone->GetChildCount());
    EXPECT_EQ(0U, clone->GetShapes().size());
    EXPECT_EQ(node->GetScale(),       clone->GetScale());
    EXPECT_EQ(node->GetRotation(),    clone->GetRotation());
    EXPECT_EQ(node->GetTranslation(), clone->GetTranslation());
}

TEST_F(CloneTest, Shapes) {
    SG::NodePtr  node  = Parser::Registry::CreateObject<SG::Node>("Node");
    SG::ShapePtr shape =
        Parser::Registry::CreateObject<SG::Shape>("Box", "TestBox");
    node->AddShape(shape);

    SG::NodePtr clone = Util::CastToDerived<SG::Node>(node->Clone(true));
    EXPECT_NOT_NULL(clone);
    EXPECT_EQ(1U, clone->GetShapes().size());

    // Shape should be a clone, not the same pointer.
    EXPECT_NE(shape,     clone->GetShapes()[0]);
    EXPECT_EQ("TestBox", clone->GetShapes()[0]->GetName());
}

TEST_F(CloneTest, Children) {
    SG::NodePtr node  = Parser::Registry::CreateObject<SG::Node>("Node");
    SG::NodePtr child = Parser::Registry::CreateObject<SG::Node>("Node",
                                                                 "Child");
    SG::NodePtr gkid  = Parser::Registry::CreateObject<SG::Node>("Node",
                                                                 "GrandKid");
    node->AddChild(child);
    child->AddChild(gkid);

    SG::NodePtr clone = Util::CastToDerived<SG::Node>(node->Clone(true));
    EXPECT_NOT_NULL(clone);
    EXPECT_EQ(1U, clone->GetChildCount());

    // Child and grandchild should be clones, not the same pointer.
    EXPECT_NE(child,   clone->GetChildren()[0]);
    EXPECT_EQ("Child", clone->GetChildren()[0]->GetName());
    EXPECT_EQ(1U, clone->GetChildren()[0]->GetChildCount());
    EXPECT_NE(gkid,       clone->GetChildren()[0]->GetChildren()[0]);
    EXPECT_EQ("GrandKid", clone->GetChildren()[0]->GetChildren()[0]->GetName());
}
