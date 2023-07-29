#include <string>

#include "Parser/Registry.h"
#include "SG/Node.h"
#include "SG/Torus.h"
#include "SG/Uniform.h"
#include "Tests/SceneTestBase.h"

class CloneTest : public SceneTestBase {};

TEST_F(CloneTest, DefaultNode) {
    SG::NodePtr node = CreateObject<SG::Node>("TestNode");
    SG::NodePtr clone = node->CloneTyped<SG::Node>(true);
    EXPECT_NOT_NULL(clone);
    EXPECT_EQ("TestNode", clone->GetName());
    EXPECT_EQ(0U, clone->GetChildCount());
    EXPECT_EQ(0U, clone->GetShapes().size());
}

TEST_F(CloneTest, BasicFields) {
    SG::NodePtr node = CreateObject<SG::Node>("Foo");
    node->SetScale(Vector3f(1, 2, 3));
    node->SetTranslation(Vector3f(10, 20, 30));
    SG::NodePtr clone = node->CloneTyped<SG::Node>(true);
    EXPECT_NOT_NULL(clone);
    EXPECT_EQ("Foo", clone->GetName());
    EXPECT_EQ(0U, clone->GetChildCount());
    EXPECT_EQ(0U, clone->GetShapes().size());
    EXPECT_EQ(node->GetScale(),       clone->GetScale());
    EXPECT_EQ(node->GetRotation(),    clone->GetRotation());
    EXPECT_EQ(node->GetTranslation(), clone->GetTranslation());
}

TEST_F(CloneTest, Shapes) {
    auto node  = CreateObject<SG::Node>();
    auto torus = CreateObject<SG::Torus>("TestTorus");
    node->AddShape(torus);

    SG::NodePtr clone = node->CloneTyped<SG::Node>(true);
    EXPECT_NOT_NULL(clone);
    EXPECT_EQ(1U, clone->GetShapes().size());

    // Shape should be a clone, not the same pointer.
    auto torus_clone =
        std::dynamic_pointer_cast<SG::Torus>(clone->GetShapes()[0]);
    EXPECT_NOT_NULL(torus_clone);
    EXPECT_NE(torus,       torus_clone);
    EXPECT_EQ("TestTorus", torus_clone->GetName());

    // The cloned Torus should have an identical mesh.
    EXPECT_EQ(torus->GetTriMesh().points,  torus_clone->GetTriMesh().points);
    EXPECT_EQ(torus->GetTriMesh().indices, torus_clone->GetTriMesh().indices);
}

TEST_F(CloneTest, Children) {
    SG::NodePtr node  = CreateObject<SG::Node>();
    SG::NodePtr child = CreateObject<SG::Node>("Child");
    SG::NodePtr gkid  = CreateObject<SG::Node>("GrandKid");
    node->AddChild(child);
    child->AddChild(gkid);

    SG::NodePtr clone = node->CloneTyped<SG::Node>(true);
    EXPECT_NOT_NULL(clone);
    EXPECT_EQ(1U, clone->GetChildCount());

    // Child and grandchild should be clones, not the same pointer.
    EXPECT_NE(child,   clone->GetChildren()[0]);
    EXPECT_EQ("Child", clone->GetChildren()[0]->GetName());
    EXPECT_EQ(1U, clone->GetChildren()[0]->GetChildCount());
    EXPECT_NE(gkid,       clone->GetChildren()[0]->GetChildren()[0]);
    EXPECT_EQ("GrandKid", clone->GetChildren()[0]->GetChildren()[0]->GetName());
}

TEST_F(CloneTest, Uniforms) {
    auto uniform = CreateObject<SG::Uniform>("uSomeName");
    uniform->SetValue<float>(4.5f);
    auto clone = uniform->CloneTyped<SG::Uniform>(true);
    EXPECT_EQ(4.5f, clone->GetFloat());
}
