#include "Commands/ChangePointTargetCommand.h"
#include "Math/Types.h"
#include "Tests/Commands/CommandTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangePointTargetCommandTest : public CommandTestBase {
  protected:
    ChangePointTargetCommandTest() {
        SetParseTypeName("ChangePointTargetCommand");
    }
};

TEST_F(ChangePointTargetCommandTest, Default) {
    auto def_pt = CreateObject<PointTarget>("Default");

    auto cptc = Command::CreateCommand<ChangePointTargetCommand>();
    EXPECT_NOT_NULL(cptc->GetOldTarget());
    EXPECT_NOT_NULL(cptc->GetNewTarget());
    EXPECT_EQ(def_pt->GetPosition(),  cptc->GetOldTarget()->GetPosition());
    EXPECT_EQ(def_pt->GetDirection(), cptc->GetOldTarget()->GetDirection());
    EXPECT_EQ(def_pt->GetRadius(),    cptc->GetOldTarget()->GetRadius());
    EXPECT_EQ(def_pt->GetArc(),       cptc->GetOldTarget()->GetArc());
}

TEST_F(ChangePointTargetCommandTest, Set) {
    auto old_pt = CreateObject<PointTarget>("Old");
    auto new_pt = CreateObject<PointTarget>("New");
    old_pt->SetPosition(Point3f(1, 2, 3));
    new_pt->SetPosition(Point3f(2, 3, 4));
    old_pt->SetDirection(Vector3f(0, 0, 1));
    new_pt->SetDirection(Vector3f(-1, 0, 0));
    old_pt->SetRadius(1);
    new_pt->SetRadius(2.4f);
    old_pt->SetArc(CircleArc(0, 90));
    new_pt->SetArc(CircleArc(-10, -20));

    auto cptc = Command::CreateCommand<ChangePointTargetCommand>();
    cptc->SetOldTarget(*old_pt);
    cptc->SetNewTarget(*new_pt);
    EXPECT_NOT_NULL(cptc->GetOldTarget());
    EXPECT_NOT_NULL(cptc->GetNewTarget());
    EXPECT_EQ(old_pt->GetPosition(),  cptc->GetOldTarget()->GetPosition());
    EXPECT_EQ(old_pt->GetDirection(), cptc->GetOldTarget()->GetDirection());
    EXPECT_EQ(new_pt->GetRadius(),    cptc->GetNewTarget()->GetRadius());
    EXPECT_EQ(new_pt->GetArc(),       cptc->GetNewTarget()->GetArc());
}

TEST_F(ChangePointTargetCommandTest, IsValid) {
    TestInvalid("", "Missing old target");
    TestInvalid("old_target: PointTarget {}", "Missing new target");
    TestValid("old_target: PointTarget{}, new_target: PointTarget {}");
}

TEST_F(ChangePointTargetCommandTest, GetDescription) {
    TestDesc("old_target: PointTarget{}, new_target: PointTarget {}",
             "Changed the point target");
}
