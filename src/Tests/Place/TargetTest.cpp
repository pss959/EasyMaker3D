#include "Place/EdgeTarget.h"
#include "Place/PointTarget.h"
#include "Tests/TestBaseWithTypes.h"
#include "Tests/Testing.h"

class TargetTest : public TestBaseWithTypes {};

TEST_F(TargetTest, PointTarget) {
    auto pt0 = CreateObject<PointTarget>("PT0");
    EXPECT_EQ("PT0",             pt0->GetName());
    EXPECT_EQ(Point3f::Zero(),   pt0->GetPosition());
    EXPECT_EQ(Vector3f::AxisY(), pt0->GetDirection());
    EXPECT_EQ(1,                 pt0->GetRadius());
    EXPECT_EQ(CircleArc(),       pt0->GetArc());

    CircleArc arc(Anglef::FromDegrees(20), Anglef::FromDegrees(-100));
    pt0->SetPosition(Point3f(1, 2, 3));
    pt0->SetDirection(Vector3f(0, 0, -4));
    pt0->SetRadius(3.5f);
    pt0->SetArc(arc);

    EXPECT_EQ(Point3f(1, 2, 3),   pt0->GetPosition());
    EXPECT_EQ(Vector3f(0, 0, -1), pt0->GetDirection());
    EXPECT_EQ(3.5f,               pt0->GetRadius());
    EXPECT_EQ(arc,                pt0->GetArc());

    auto pt1 = CreateObject<PointTarget>("PT1");
    pt1->CopyFrom(*pt0);
    EXPECT_EQ("PT1",               pt1->GetName());
    EXPECT_EQ(pt0->GetPosition(),  pt1->GetPosition());
    EXPECT_EQ(pt0->GetDirection(), pt1->GetDirection());
    EXPECT_EQ(pt0->GetRadius(),    pt1->GetRadius());
    EXPECT_EQ(pt0->GetArc(),       pt1->GetArc());
}

TEST_F(TargetTest, EdgeTarget) {
    auto et0 = CreateObject<EdgeTarget>("ET0");
    EXPECT_EQ("ET0",             et0->GetName());
    EXPECT_EQ(Point3f::Zero(),   et0->GetPosition0());
    EXPECT_EQ(Point3f(0, 4, 0),  et0->GetPosition1());
    EXPECT_EQ(Vector3f::AxisY(), et0->GetDirection());
    EXPECT_EQ(4,                 et0->GetLength());

    et0->SetPositions(Point3f(1, 2, 3), Point3f(7, 2, 3));
    EXPECT_EQ(Point3f(1, 2, 3),  et0->GetPosition0());
    EXPECT_EQ(Point3f(7, 2, 3),  et0->GetPosition1());
    EXPECT_EQ(Vector3f(1, 0, 0), et0->GetDirection());
    EXPECT_EQ(6,                 et0->GetLength());

    auto et1 = CreateObject<EdgeTarget>("ET1");
    et1->CopyFrom(*et0);
    EXPECT_EQ("ET1",               et1->GetName());
    EXPECT_EQ(et0->GetPosition0(), et1->GetPosition0());
    EXPECT_EQ(et0->GetPosition1(), et1->GetPosition1());
    EXPECT_EQ(et0->GetDirection(), et1->GetDirection());
    EXPECT_EQ(et0->GetLength(),    et1->GetLength());
}
