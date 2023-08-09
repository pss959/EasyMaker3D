#include <ion/math/vectorutils.h>

#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Util/Tuning.h"
#include "Items/Grippable.h"

/// \ingroup Tests
class GrippableTest : public SceneTestBase {
  protected:
    /// Derived Grippable class used to test the interface.
    class TestGrippable : public Grippable {
      public:
        bool grip_activated = false;

        virtual const SG::Node * GetGrippableNode() const override {
            return node_.get();
        }
        virtual void UpdateGripInfo(GripInfo &info) override {
            // XXXX
        }
        virtual void ActivateGrip(Hand hand, bool is_active) override {
            Grippable::ActivateGrip(hand, is_active);
            grip_activated = is_active;
        }

        /// For testing GetBestAxis() and GetBestDirIndex().
        static int GetAxis(const Vector3f &dir, bool &is_opposite) {
            return GetBestAxis(ion::math::Normalized(dir),
                               Anglef::FromDegrees(5), is_opposite);
        }

        // Make these publicly callable.
        using Grippable::GetPath;
        using Grippable::GetNeutralGripColor;
        using Grippable::GetActiveGripColor;
      protected:
        TestGrippable() {
            node_ = CreateObject<SG::Node>("TestNode");
            SetPath(SG::NodePath(node_));
        }
      private:
        SG::NodePtr node_;
        friend class Parser::Registry;
    };

    GrippableTest() {
        Parser::Registry::AddType<TestGrippable>("TestGrippable");
    }
};

TEST_F(GrippableTest, Grippable) {
    auto tg = CreateObject<TestGrippable>();
    EXPECT_EQ(1U,         tg->GetPath().size());
    EXPECT_EQ("TestNode", tg->GetPath()[0]->GetName());

    EXPECT_EQ(SG::ColorMap::SGetColor("GripNeutralColor"),
              tg->GetNeutralGripColor());
    EXPECT_EQ(SG::ColorMap::SGetColor("GripActiveColor"),
              tg->GetActiveGripColor());

    EXPECT_FALSE(tg->grip_activated);
    tg->ActivateGrip(Hand::kLeft, true);
    EXPECT_TRUE(tg->grip_activated);
    tg->ActivateGrip(Hand::kLeft, false);
    EXPECT_FALSE(tg->grip_activated);

    bool is_opposite;
    EXPECT_EQ(-1, tg->GetAxis(Vector3f(1, 1, 1), is_opposite));
    EXPECT_EQ(0, tg->GetAxis(Vector3f(1, .01f, .01f), is_opposite));
    EXPECT_FALSE(is_opposite);
    EXPECT_EQ(0, tg->GetAxis(Vector3f(-1, .01f, .01f), is_opposite));
    EXPECT_TRUE(is_opposite);
    EXPECT_EQ(1, tg->GetAxis(Vector3f(.01f, .9f, .01f), is_opposite));
    EXPECT_FALSE(is_opposite);
    EXPECT_EQ(1, tg->GetAxis(Vector3f(.01f, -.9f, .01f), is_opposite));
    EXPECT_TRUE(is_opposite);
    EXPECT_EQ(2, tg->GetAxis(Vector3f(.01f, .01f, .9f), is_opposite));
    EXPECT_FALSE(is_opposite);
    EXPECT_EQ(2, tg->GetAxis(Vector3f(.01f, .01f, -.9f), is_opposite));
    EXPECT_TRUE(is_opposite);
}
