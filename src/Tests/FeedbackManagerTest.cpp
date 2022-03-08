#include <memory>

#include "Feedback/Feedback.h"
#include "Managers/FeedbackManager.h"
#include "Parser/Registry.h"
#include "SceneTestBase.h"

// Derived Feedback classes to allow testing.
class TestFeedback : public Feedback {
  public:
    bool is_active = false;
    virtual void SetColor(const Color &color) override {}
    virtual void Activate()   override { is_active = true;  }
    virtual void Deactivate() override { is_active = false; }
};
class AFeedback : public TestFeedback {
  protected:
    AFeedback() {}
    friend class Parser::Registry;
};
class BFeedback : public TestFeedback {
  protected:
    BFeedback() {}
    friend class Parser::Registry;
};
typedef std::shared_ptr<AFeedback> AFeedbackPtr;
typedef std::shared_ptr<BFeedback> BFeedbackPtr;

class FeedbackManagerTest : public SceneTestBase {};

TEST_F(FeedbackManagerTest, ActivateDeactivate) {
    Parser::Registry::AddType<AFeedback>("AFeedback");
    Parser::Registry::AddType<BFeedback>("BFeedback");

    AFeedbackPtr atmp = CreateObject<AFeedback>("AFeedback");
    BFeedbackPtr btmp = CreateObject<BFeedback>("BFeedback");

    SG::NodePtr world_parent(CreateObject<SG::Node>("WorldParent"));
    SG::NodePtr stage_parent(CreateObject<SG::Node>("StageParent"));

    // Initialize the FeedbackManager and add the templates.
    FeedbackManager fm;
    fm.SetParentNodes(world_parent, stage_parent);
    fm.SetPathToStage(SG::NodePath(world_parent));  // Path does not matter.
    fm.AddOriginal<AFeedback>(atmp);
    fm.AddOriginal<BFeedback>(btmp);
    EXPECT_FALSE(atmp->is_active);
    EXPECT_FALSE(btmp->is_active);
    EXPECT_EQ(0U, stage_parent->GetChildCount());

    // Activate an AFeedback instance.
    AFeedbackPtr a0 = fm.Activate<AFeedback>();
    EXPECT_NOT_NULL(a0);
    EXPECT_TRUE(a0->is_active);
    EXPECT_FALSE(atmp->is_active);
    EXPECT_FALSE(btmp->is_active);
    EXPECT_EQ(1U, stage_parent->GetChildCount());
    EXPECT_EQ(a0, stage_parent->GetChildren()[0]);

    // Activate another AFeedback instance.
    AFeedbackPtr a1 = fm.Activate<AFeedback>();
    EXPECT_NOT_NULL(a1);
    EXPECT_TRUE(a0->is_active);
    EXPECT_TRUE(a1->is_active);
    EXPECT_FALSE(atmp->is_active);
    EXPECT_FALSE(btmp->is_active);
    EXPECT_EQ(2U, stage_parent->GetChildCount());
    EXPECT_EQ(a0, stage_parent->GetChildren()[0]);
    EXPECT_EQ(a1, stage_parent->GetChildren()[1]);

    // Activate a BFeedback instance.
    BFeedbackPtr b0 = fm.Activate<BFeedback>();
    EXPECT_NOT_NULL(b0);
    EXPECT_TRUE(a0->is_active);
    EXPECT_TRUE(a1->is_active);
    EXPECT_TRUE(b0->is_active);
    EXPECT_FALSE(atmp->is_active);
    EXPECT_FALSE(btmp->is_active);
    EXPECT_EQ(3U, stage_parent->GetChildCount());
    EXPECT_EQ(a0, stage_parent->GetChildren()[0]);
    EXPECT_EQ(a1, stage_parent->GetChildren()[1]);
    EXPECT_EQ(b0, stage_parent->GetChildren()[2]);

    // Deactivate a0.
    fm.Deactivate(a0);
    EXPECT_FALSE(a0->is_active);
    EXPECT_TRUE(a1->is_active);
    EXPECT_TRUE(b0->is_active);
    EXPECT_FALSE(atmp->is_active);
    EXPECT_FALSE(btmp->is_active);
    EXPECT_EQ(2U, stage_parent->GetChildCount());
    EXPECT_EQ(a1, stage_parent->GetChildren()[0]);
    EXPECT_EQ(b0, stage_parent->GetChildren()[1]);

    // Activate another AFeedback - it should reuse a0.
    AFeedbackPtr a2 = fm.Activate<AFeedback>();
    EXPECT_NOT_NULL(a2);
    EXPECT_EQ(a0, a2);
    EXPECT_TRUE(a1->is_active);
    EXPECT_TRUE(a2->is_active);
    EXPECT_TRUE(b0->is_active);
    EXPECT_FALSE(atmp->is_active);
    EXPECT_FALSE(btmp->is_active);
    EXPECT_EQ(3U, stage_parent->GetChildCount());
    EXPECT_EQ(a1, stage_parent->GetChildren()[0]);
    EXPECT_EQ(b0, stage_parent->GetChildren()[1]);
    EXPECT_EQ(a2, stage_parent->GetChildren()[2]);

    // Deactivate everything.
    fm.Deactivate(a1);
    fm.Deactivate(a2);
    fm.Deactivate(b0);
    EXPECT_FALSE(a1->is_active);
    EXPECT_FALSE(a2->is_active);
    EXPECT_FALSE(b0->is_active);
    EXPECT_FALSE(atmp->is_active);
    EXPECT_FALSE(btmp->is_active);
    EXPECT_EQ(0U, stage_parent->GetChildCount());
}
