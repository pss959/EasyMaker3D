#include "Math/Profile.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

class ProfileTest : public TestBase {
};

TEST_F(ProfileTest, Default) {
    // The default constructor creates an invalid Profile.
    Profile p;
    EXPECT_FALSE(p.IsValid());
    EXPECT_EQ(0U, p.GetMinPointCount());
}

TEST_F(ProfileTest, Fixed0) {
    const Point2f p0(0, 1);
    const Point2f p1(1, 0);

    // Create a fixed profile with no interior points.
    Profile p(Profile::Type::kFixed, 2, Profile::PointVec{p0, p1});
    EXPECT_TRUE(p.IsValid());
    EXPECT_EQ(Profile::Type::kFixed, p.GetType());
    EXPECT_EQ(2U, p.GetMinPointCount());
    EXPECT_EQ(2U, p.GetPointCount());
    EXPECT_EQ(0U, p.GetMovablePointCount());
    const auto &pp = p.GetPoints();
    EXPECT_EQ(2U, pp.size());
    EXPECT_EQ(p0, pp[0]);
    EXPECT_EQ(p1, pp[1]);

    EXPECT_TRUE(p.IsFixedPoint(0));
    EXPECT_TRUE(p.IsFixedPoint(1));
    TEST_THROW(p.IsFixedPoint(2), AssertException, "index");

    EXPECT_EQ(p0, p.GetPreviousPoint(1));
    EXPECT_EQ(p1, p.GetNextPoint(0));
    TEST_THROW(p.GetPreviousPoint(0), AssertException, "index");
    TEST_THROW(p.GetNextPoint(1),     AssertException, "index");
}

TEST_F(ProfileTest, Fixed1) {
    // Create a fixed profile with 1 interior point.
    const Point2f p0(0, 1);
    const Point2f p1(0, 0);
    const Point2f pm(1, .5f);

    Profile p = Profile::CreateFixedProfile(p0, p1, 3, Profile::PointVec{ pm });
    EXPECT_TRUE(p.IsValid());
    EXPECT_EQ(Profile::Type::kFixed, p.GetType());
    EXPECT_EQ(3U, p.GetMinPointCount());
    EXPECT_EQ(3U, p.GetPointCount());
    EXPECT_EQ(1U, p.GetMovablePointCount());
    const auto pp = p.GetPoints();
    EXPECT_EQ(3U, pp.size());
    EXPECT_EQ(p0, pp[0]);
    EXPECT_EQ(pm, pp[1]);
    EXPECT_EQ(p1, pp[2]);

    EXPECT_TRUE(p.IsFixedPoint(0));
    EXPECT_FALSE(p.IsFixedPoint(1));
    EXPECT_TRUE(p.IsFixedPoint(2));
    TEST_THROW(p.IsFixedPoint(3), AssertException, "index");

    EXPECT_EQ(p0, p.GetPreviousPoint(1));
    EXPECT_EQ(pm, p.GetPreviousPoint(2));
    EXPECT_EQ(pm, p.GetNextPoint(0));
    EXPECT_EQ(p1, p.GetNextPoint(1));
    TEST_THROW(p.GetPreviousPoint(0), AssertException, "index");
    TEST_THROW(p.GetNextPoint(2),     AssertException, "index");
}

TEST_F(ProfileTest, Closed) {
    // Create a closed profile with 4 points.
    const Profile::PointVec pts{
        Point2f(0, 0), Point2f(1, 0),
        Point2f(1, 1), Point2f(0, 1)
    };
    Profile p(Profile::Type::kClosed, 3, pts);
    EXPECT_TRUE(p.IsValid());
    EXPECT_EQ(Profile::Type::kClosed, p.GetType());
    EXPECT_EQ(4U,  p.GetPointCount());
    EXPECT_EQ(4U,  p.GetMovablePointCount());
    EXPECT_EQ(pts, p.GetPoints());
    EXPECT_EQ(pts, p.GetMovablePoints());

    EXPECT_FALSE(p.IsFixedPoint(0));
    EXPECT_FALSE(p.IsFixedPoint(1));
    EXPECT_FALSE(p.IsFixedPoint(2));
    EXPECT_FALSE(p.IsFixedPoint(3));
    TEST_THROW(p.IsFixedPoint(4), AssertException, "index");

    EXPECT_EQ(pts[3], p.GetPreviousPoint(0));
    EXPECT_EQ(pts[0], p.GetPreviousPoint(1));
    EXPECT_EQ(pts[1], p.GetPreviousPoint(2));
    EXPECT_EQ(pts[2], p.GetPreviousPoint(3));
    EXPECT_EQ(pts[1], p.GetNextPoint(0));
    EXPECT_EQ(pts[2], p.GetNextPoint(1));
    EXPECT_EQ(pts[3], p.GetNextPoint(2));
    EXPECT_EQ(pts[0], p.GetNextPoint(3));
    TEST_THROW(p.GetPreviousPoint(4), AssertException, "index");
    TEST_THROW(p.GetNextPoint(4),     AssertException, "index");

    // Append a point.
    const Point2f p4(.5f, .5f);
    p.AppendPoint(p4);
    EXPECT_EQ(5U,     p.GetPointCount());
    EXPECT_EQ(5U,     p.GetMovablePointCount());
    EXPECT_EQ(pts[0], p.GetPoints()[0]);
    EXPECT_EQ(pts[1], p.GetPoints()[1]);
    EXPECT_EQ(pts[2], p.GetPoints()[2]);
    EXPECT_EQ(pts[3], p.GetPoints()[3]);
    EXPECT_EQ(p4,     p.GetPoints()[4]);
    EXPECT_EQ(p.GetPoints(), p.GetMovablePoints());
    EXPECT_FALSE(p.IsFixedPoint(0));
    EXPECT_FALSE(p.IsFixedPoint(1));
    EXPECT_FALSE(p.IsFixedPoint(2));
    EXPECT_FALSE(p.IsFixedPoint(3));
    EXPECT_FALSE(p.IsFixedPoint(4));
    TEST_THROW(p.IsFixedPoint(5), AssertException, "index");

    // Append another point by inserting at the end.
    p.InsertPoint(5U, Point2f(.3f, .3f));
    EXPECT_TRUE(p.IsValid());
    EXPECT_EQ(6U,     p.GetPointCount());
    EXPECT_EQ(6U,     p.GetMovablePointCount());
    EXPECT_EQ(p.GetPoints(), p.GetMovablePoints());
}

TEST_F(ProfileTest, Edit) {
    const Point2f p0(0, 1);
    const Point2f p1(1, 0);

    // Create a fixed profile with 1 interior point.
    Profile p = Profile::CreateFixedProfile(
        p0, p1, 3, Profile::PointVec{ Point2f(1, .5f) });

    // Move the interior point.
    p.SetPoint(1, Point2f(.5f, .5f));
    EXPECT_EQ(3U, p.GetPointCount());
    EXPECT_EQ(1U, p.GetMovablePointCount());
    EXPECT_EQ(Point2f(.5f, .5f), p.GetPoints()[1]);

    // Add an interior point.
    p.AppendPoint(Point2f(.2f, .2f));
    EXPECT_TRUE(p.IsValid());
    EXPECT_EQ(4U, p.GetPointCount());
    EXPECT_EQ(2U, p.GetMovablePointCount());
    EXPECT_EQ(p0,                p.GetPoints()[0]);
    EXPECT_EQ(Point2f(.5f, .5f), p.GetPoints()[1]);
    EXPECT_EQ(Point2f(.2f, .2f), p.GetPoints()[2]);
    EXPECT_EQ(p1,                p.GetPoints()[3]);

    // Insert another interior point.
    p.InsertPoint(2, Point2f(.4f, .0));
    EXPECT_TRUE(p.IsValid());
    EXPECT_EQ(5U, p.GetPointCount());
    EXPECT_EQ(3U, p.GetMovablePointCount());
    EXPECT_EQ(p0,                p.GetPoints()[0]);
    EXPECT_EQ(Point2f(.5f, .5f), p.GetPoints()[1]);
    EXPECT_EQ(Point2f(.4f, .0),  p.GetPoints()[2]);
    EXPECT_EQ(Point2f(.2f, .2f), p.GetPoints()[3]);
    EXPECT_EQ(p1,                p.GetPoints()[4]);

    // Remove it.
    p.RemovePoint(2);
    EXPECT_TRUE(p.IsValid());
    EXPECT_EQ(4U, p.GetPointCount());
    EXPECT_EQ(2U, p.GetMovablePointCount());
    EXPECT_EQ(p0,                p.GetPoints()[0]);
    EXPECT_EQ(Point2f(.5f, .5f), p.GetPoints()[1]);
    EXPECT_EQ(Point2f(.2f, .2f), p.GetPoints()[2]);
    EXPECT_EQ(p1,                p.GetPoints()[3]);

    // Remove the first movable point.
    p.RemovePoint(1);
    EXPECT_TRUE(p.IsValid());
    EXPECT_EQ(3U, p.GetPointCount());
    EXPECT_EQ(1U, p.GetMovablePointCount());
    EXPECT_EQ(p0,                p.GetPoints()[0]);
    EXPECT_EQ(Point2f(.2f, .2f), p.GetPoints()[1]);
    EXPECT_EQ(p1,                p.GetPoints()[2]);

    // Test error cases:
    TEST_THROW(p.RemovePoint(0), AssertException, "Fixed");
    TEST_THROW(p.RemovePoint(2), AssertException, "Fixed");
    TEST_THROW(p.RemovePoint(1), AssertException, "min_count");
    TEST_THROW(p.SetPoint(0,    Point2f(.5f, .5f)), AssertException, "Fixed");
    TEST_THROW(p.SetPoint(2,    Point2f(.5f, .5f)), AssertException, "Fixed");
    TEST_THROW(p.SetPoint(3,    Point2f(.5f, .5f)), AssertException, "index");
    TEST_THROW(p.InsertPoint(3, Point2f(.5f, .5f)), AssertException, "index");
}
