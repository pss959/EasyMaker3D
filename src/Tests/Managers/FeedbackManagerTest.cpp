//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include <memory>

#include "Feedback/Feedback.h"
#include "Managers/FeedbackManager.h"
#include "Parser/Registry.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing2.h"
#include "Util/Assert.h"

/// \ingroup Tests
class FeedbackManagerTest : public SceneTestBase {
  protected:
    /// Derived Feedback class for testing FeedbackManager to Feedback interface.
    class TestFeedback : public Feedback {
      public:
        bool is_active = false;
        virtual void SetColor(const Color &color) override {}
        virtual void Activate()   override { is_active = true;  }
        virtual void Deactivate() override { is_active = false; }
        using Feedback::GetSceneBounds;  // Makes this public.
    };

    class AFeedback : public TestFeedback {
      protected:
        AFeedback() {}
        friend class Parser::Registry;
    };

    class BFeedback : public TestFeedback {
      protected:
        BFeedback() {}
        virtual bool IsInWorldCoordinates() const override { return true; }
        friend class Parser::Registry;
    };

    DECL_SHARED_PTR(AFeedback);
    DECL_SHARED_PTR(BFeedback);

    FeedbackManager fm;

    AFeedbackPtr atmp;
    BFeedbackPtr btmp;

    SG::NodePtr world_parent;
    SG::NodePtr stage_parent;

    /// The constructor sets up the AFeedback and BFeedback classes.
    FeedbackManagerTest();
};

FeedbackManagerTest::FeedbackManagerTest() {
    Parser::Registry::AddType<AFeedback>("AFeedback");
    Parser::Registry::AddType<BFeedback>("BFeedback");

    atmp = CreateObject<AFeedback>("AFeedback");
    btmp = CreateObject<BFeedback>("BFeedback");

    world_parent = CreateObject<SG::Node>("WorldParent");
    stage_parent = CreateObject<SG::Node>("StageParent");

    // Initialize the FeedbackManager and add the templates.
    fm.SetParentNodes(world_parent, stage_parent);
    fm.SetPathToStage(SG::NodePath(world_parent));  // Path does not matter.
    fm.AddOriginal<AFeedback>(atmp);
    fm.AddOriginal<BFeedback>(btmp);
    EXPECT_FALSE(atmp->is_active);
    EXPECT_FALSE(btmp->is_active);
    EXPECT_EQ(0U, stage_parent->GetChildCount());
}

TEST_F(FeedbackManagerTest, ActivateDeactivate) {
    // Activate an AFeedback instance.
    auto a0 = fm.Activate<AFeedback>();
    EXPECT_NOT_NULL(a0);
    EXPECT_TRUE(a0->is_active);
    EXPECT_FALSE(atmp->is_active);
    EXPECT_FALSE(btmp->is_active);
    EXPECT_EQ(1U, stage_parent->GetChildCount());
    EXPECT_EQ(a0, stage_parent->GetChildren()[0]);
    EXPECT_EQ(0U, world_parent->GetChildCount());

    // Activate another AFeedback instance.
    auto a1 = fm.Activate<AFeedback>();
    EXPECT_NOT_NULL(a1);
    EXPECT_TRUE(a0->is_active);
    EXPECT_TRUE(a1->is_active);
    EXPECT_FALSE(atmp->is_active);
    EXPECT_FALSE(btmp->is_active);
    EXPECT_EQ(2U, stage_parent->GetChildCount());
    EXPECT_EQ(a0, stage_parent->GetChildren()[0]);
    EXPECT_EQ(a1, stage_parent->GetChildren()[1]);
    EXPECT_EQ(0U, world_parent->GetChildCount());

    // Activate a BFeedback instance. Should be added to world_parent.
    auto b0 = fm.Activate<BFeedback>();
    EXPECT_NOT_NULL(b0);
    EXPECT_TRUE(a0->is_active);
    EXPECT_TRUE(a1->is_active);
    EXPECT_TRUE(b0->is_active);
    EXPECT_FALSE(atmp->is_active);
    EXPECT_FALSE(btmp->is_active);
    EXPECT_EQ(2U, stage_parent->GetChildCount());
    EXPECT_EQ(a0, stage_parent->GetChildren()[0]);
    EXPECT_EQ(a1, stage_parent->GetChildren()[1]);
    EXPECT_EQ(1U, world_parent->GetChildCount());
    EXPECT_EQ(b0, world_parent->GetChildren()[0]);

    // Deactivate a0.
    fm.Deactivate(a0);
    EXPECT_FALSE(a0->is_active);
    EXPECT_TRUE(a1->is_active);
    EXPECT_TRUE(b0->is_active);
    EXPECT_FALSE(atmp->is_active);
    EXPECT_FALSE(btmp->is_active);
    EXPECT_EQ(1U, stage_parent->GetChildCount());
    EXPECT_EQ(a1, stage_parent->GetChildren()[0]);
    EXPECT_EQ(1U, world_parent->GetChildCount());
    EXPECT_EQ(b0, world_parent->GetChildren()[0]);

    // Activate another AFeedback - it should reuse a0.
    auto a2 = fm.Activate<AFeedback>();
    EXPECT_NOT_NULL(a2);
    EXPECT_EQ(a0, a2);
    EXPECT_TRUE(a1->is_active);
    EXPECT_TRUE(a2->is_active);
    EXPECT_TRUE(b0->is_active);
    EXPECT_FALSE(atmp->is_active);
    EXPECT_FALSE(btmp->is_active);
    EXPECT_EQ(2U, stage_parent->GetChildCount());
    EXPECT_EQ(a1, stage_parent->GetChildren()[0]);
    EXPECT_EQ(a2, stage_parent->GetChildren()[1]);
    EXPECT_EQ(1U, world_parent->GetChildCount());
    EXPECT_EQ(b0, world_parent->GetChildren()[0]);

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
    EXPECT_EQ(0U, world_parent->GetChildCount());
}

TEST_F(FeedbackManagerTest, ActivateDeactivateWithKey) {
    auto a = fm.ActivateWithKey<AFeedback>("SomeKeyA");
    auto b = fm.ActivateWithKey<BFeedback>("SomeKeyB");

    // No duplicate keys.
    TEST_ASSERT(fm.ActivateWithKey<AFeedback>("SomeKeyA"), "contains(key)");

    // Wrong type for key.
    TEST_ASSERT(fm.DeactivateWithKey<BFeedback>("SomeKeyA"), "instance");

    fm.DeactivateWithKey<AFeedback>("SomeKeyA");
    fm.DeactivateWithKey<BFeedback>("SomeKeyB");

    // Key no longer exists.
    TEST_ASSERT(fm.DeactivateWithKey<AFeedback>("SomeKeyA"), "instance");
}

TEST_F(FeedbackManagerTest, SceneBounds) {
    auto a0  = fm.Activate<AFeedback>();
    auto b0 = fm.ActivateWithKey<BFeedback>("BKey");

    // No function set.
    TEST_ASSERT(a0->GetSceneBounds(), "scene_bounds_func_");

    // Setting function should update existing active instances.
    auto sb_func = [](){ return Bounds(Vector3f(10, 20, 30)); };
    fm.SetSceneBoundsFunc(sb_func);
    EXPECT_EQ(Vector3f(10, 20, 30), a0->GetSceneBounds().GetSize());
    EXPECT_EQ(Vector3f(10, 20, 30), b0->GetSceneBounds().GetSize());

    // Same for new instance.
    auto b1 = fm.Activate<BFeedback>();
    EXPECT_EQ(Vector3f(10, 20, 30), b1->GetSceneBounds().GetSize());
}
