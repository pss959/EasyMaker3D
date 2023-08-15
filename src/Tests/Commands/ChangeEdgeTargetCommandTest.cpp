#include "Commands/ChangeEdgeTargetCommand.h"
#include "Math/Types.h"
#include "Tests/Commands/CommandTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeEdgeTargetCommandTest : public CommandTestBase {
  protected:
    ChangeEdgeTargetCommandTest() {
        SetParseTypeName("ChangeEdgeTargetCommand");
    }
};

TEST_F(ChangeEdgeTargetCommandTest, Default) {
    auto cetc = Command::CreateCommand<ChangeEdgeTargetCommand>();
    EXPECT_NOT_NULL(cetc->GetOldTarget());
    EXPECT_NOT_NULL(cetc->GetNewTarget());
    EXPECT_EQ(Point3f(0, 0, 0), cetc->GetOldTarget()->GetPosition0());
    EXPECT_EQ(Point3f(0, 4, 0), cetc->GetOldTarget()->GetPosition1());
    EXPECT_EQ(Point3f(0, 0, 0), cetc->GetNewTarget()->GetPosition0());
    EXPECT_EQ(Point3f(0, 4, 0), cetc->GetNewTarget()->GetPosition1());
}

TEST_F(ChangeEdgeTargetCommandTest, Set) {
    auto old_et = CreateObject<EdgeTarget>("Old");
    auto new_et = CreateObject<EdgeTarget>("New");
    old_et->SetPositions(Point3f(1, 2, 3), Point3f(4, 5, 6));
    new_et->SetPositions(Point3f(2, 3, 4), Point3f(5, 6, 7));

    auto cetc = Command::CreateCommand<ChangeEdgeTargetCommand>();
    cetc->SetOldTarget(*old_et);
    cetc->SetNewTarget(*new_et);
    EXPECT_NOT_NULL(cetc->GetOldTarget());
    EXPECT_NOT_NULL(cetc->GetNewTarget());
    EXPECT_EQ(old_et->GetPosition0(), cetc->GetOldTarget()->GetPosition0());
    EXPECT_EQ(old_et->GetPosition1(), cetc->GetOldTarget()->GetPosition1());
    EXPECT_EQ(new_et->GetPosition0(), cetc->GetNewTarget()->GetPosition0());
    EXPECT_EQ(new_et->GetPosition1(), cetc->GetNewTarget()->GetPosition1());
}

TEST_F(ChangeEdgeTargetCommandTest, IsValid) {
    TestInvalid("", "Missing old target");
    TestInvalid("old_target: EdgeTarget {}", "Missing new target");
    TestValid(R"(old_target: EdgeTarget {}, new_target: EdgeTarget {})");
}

TEST_F(ChangeEdgeTargetCommandTest, GetDescription) {
    TestDesc(R"(old_target: EdgeTarget{}, new_target: EdgeTarget {})",
             "Changed the edge target");
}
