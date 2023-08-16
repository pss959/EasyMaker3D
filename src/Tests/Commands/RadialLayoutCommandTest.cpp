#include "Commands/RadialLayoutCommand.h"
#include "Place/PointTarget.h"
#include "Tests/Commands/CommandTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class RadialLayoutCommandTest : public CommandTestBase {
  protected:
    RadialLayoutCommandTest() { SetParseTypeName("RadialLayoutCommand"); }
};

TEST_F(RadialLayoutCommandTest, Default) {
    auto rlc = Command::CreateCommand<RadialLayoutCommand>();
    EXPECT_EQ(Point3f::Zero(),   rlc->GetCenter());
    EXPECT_EQ(Vector3f::AxisY(), rlc->GetNormal());
    EXPECT_EQ(1,                 rlc->GetRadius());
    EXPECT_EQ(CircleArc(),       rlc->GetArc());
}

TEST_F(RadialLayoutCommandTest, Set) {
    // Set up a PointTarget for setting values.
    auto pt = CreateObject<PointTarget>();
    pt->SetPosition(Point3f(1, 2, 3));
    pt->SetDirection(Vector3f(0, 0, 1));
    pt->SetRadius(2.5f);
    pt->SetArc(CircleArc(20, -140));

    auto rlc = Command::CreateCommand<RadialLayoutCommand>();
    rlc->SetFromTarget(*pt);
    EXPECT_EQ(Point3f(1, 2, 3),    rlc->GetCenter());
    EXPECT_EQ(Vector3f(0, 0, 1),   rlc->GetNormal());
    EXPECT_EQ(2.5f,                rlc->GetRadius());
    EXPECT_EQ(CircleArc(20, -140), rlc->GetArc());
}

TEST_F(RadialLayoutCommandTest, IsValid) {
    TestInvalid("", "Missing model names");
    TestInvalid(R"(model_names: [ " BadName" ])", "Invalid model name");
    TestValid(R"(model_names: ["Box"], center: 1 2 -3)");
}

TEST_F(RadialLayoutCommandTest, GetDescription) {
    TestDesc(R"(model_names: ["Box"])",   R"(Radial layout of Model "Box")");
    TestDesc(R"(model_names: ["A", "B"])",  "Radial layout of 2 Models");
}
