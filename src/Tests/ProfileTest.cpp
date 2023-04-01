#include "Math/Profile.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

class ProfileTest : public TestBase {
};

TEST_F(ProfileTest, Default) {
    // The default constructor creates an invalid Profile.
    Profile p;
    EXPECT_FALSE(p.IsValid());
}

TEST_F(ProfileTest, Open0) {
    const Point2f p0(0, 1);
    const Point2f p1(1, 0);

    // Create an open profile with no interior points.
    Profile p(p0, p1);
    EXPECT_TRUE(p.IsValid());
    EXPECT_TRUE(p.IsOpen());
    EXPECT_EQ(p0, p.GetStartPoint());
    EXPECT_EQ(p1, p.GetEndPoint());
    EXPECT_EQ(0U, p.GetPointCount());
    EXPECT_EQ(2U, p.GetTotalPointCount());
    const auto &pp = p.GetPoints();
    EXPECT_TRUE(pp.empty());
    const auto ap = p.GetAllPoints();
    EXPECT_EQ(2U, ap.size());
    EXPECT_EQ(p0, ap[0]);
    EXPECT_EQ(p1, ap[1]);

    TEST_THROW(p.GetPreviousPoint(0), AssertException, "index");
    TEST_THROW(p.GetNextPoint(0),     AssertException, "index");
}

TEST_F(ProfileTest, Open1) {
    // Create an open profile with 1 interior point.
    const Point2f p0(0, 1);
    const Point2f p1(0, 0);
    const Point2f pm(1, .5f);

    Profile p(p0, p1, Profile::PointVec{ pm }, 1);
    EXPECT_TRUE(p.IsValid());
    EXPECT_TRUE(p.IsOpen());
    EXPECT_EQ(p0, p.GetStartPoint());
    EXPECT_EQ(p1, p.GetEndPoint());
    EXPECT_EQ(1U, p.GetPointCount());
    EXPECT_EQ(3U, p.GetTotalPointCount());
    const auto &pp = p.GetPoints();
    EXPECT_EQ(1U, pp.size());
    EXPECT_EQ(pm, pp[0]);
    const auto ap = p.GetAllPoints();
    EXPECT_EQ(3U, ap.size());
    EXPECT_EQ(p0, ap[0]);
    EXPECT_EQ(pm, ap[1]);
    EXPECT_EQ(p1, ap[2]);

    EXPECT_EQ(p0, p.GetPreviousPoint(0));
    EXPECT_EQ(p1, p.GetNextPoint(0));

    TEST_THROW(p.GetPreviousPoint(1), AssertException, "index");
    TEST_THROW(p.GetNextPoint(1),     AssertException, "index");
}

TEST_F(ProfileTest, Closed) {
    // Create a closed profile with 4 points.
    const Profile::PointVec pts{
        Point2f(0, 0), Point2f(1, 0),
        Point2f(1, 1), Point2f(0, 1)
    };
    Profile p(pts, 3);
    EXPECT_TRUE(p.IsValid());
    EXPECT_FALSE(p.IsOpen());
    EXPECT_EQ(4U,  p.GetPointCount());
    EXPECT_EQ(4U,  p.GetTotalPointCount());
    EXPECT_EQ(pts, p.GetPoints());
    EXPECT_EQ(pts, p.GetAllPoints());

    TEST_THROW(p.GetStartPoint(), AssertException, "IsOpen");
    TEST_THROW(p.GetEndPoint(),   AssertException, "IsOpen");

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
}

TEST_F(ProfileTest, Edit) {
    // Create an open profile with 1 interior point.
    Profile p(Point2f(0, 1), Point2f(1, 0),
              Profile::PointVec{ Point2f(1, .5f) }, 1);

    // Move the interior point.
    p.SetPoint(0, Point2f(.5f, .5f));
    EXPECT_EQ(1U, p.GetPointCount());
    EXPECT_EQ(Point2f(.5f, .5f), p.GetPoints()[0]);

    // Add an interior point.
    p.AddPoint(Point2f(.2f, .2f));
    EXPECT_TRUE(p.IsValid());
    EXPECT_EQ(2U, p.GetPointCount());
    EXPECT_EQ(4U, p.GetTotalPointCount());
    EXPECT_EQ(Point2f(0, 1), p.GetStartPoint());
    EXPECT_EQ(Point2f(1, 0), p.GetEndPoint());
    EXPECT_EQ(Point2f(.5f, .5f), p.GetPoints()[0]);
    EXPECT_EQ(Point2f(.2f, .2f), p.GetPoints()[1]);

    // Insert an interior point.
    p.InsertPoint(1, Point2f(.4f, .0));
    EXPECT_TRUE(p.IsValid());
    EXPECT_EQ(3U, p.GetPointCount());
    EXPECT_EQ(5U, p.GetTotalPointCount());
    EXPECT_EQ(Point2f(0, 1), p.GetStartPoint());
    EXPECT_EQ(Point2f(1, 0), p.GetEndPoint());
    EXPECT_EQ(Point2f(.5f, .5f), p.GetPoints()[0]);
    EXPECT_EQ(Point2f(.4f, .0),  p.GetPoints()[1]);
    EXPECT_EQ(Point2f(.2f, .2f), p.GetPoints()[2]);

    // Remove it.
    p.RemovePoint(1);
    EXPECT_TRUE(p.IsValid());
    EXPECT_EQ(2U, p.GetPointCount());
    EXPECT_EQ(4U, p.GetTotalPointCount());
    EXPECT_EQ(Point2f(0, 1), p.GetStartPoint());
    EXPECT_EQ(Point2f(1, 0), p.GetEndPoint());
    EXPECT_EQ(Point2f(.5f, .5f), p.GetPoints()[0]);
    EXPECT_EQ(Point2f(.2f, .2f), p.GetPoints()[1]);

    // Remove the first one.
    p.RemovePoint(0);
    EXPECT_TRUE(p.IsValid());
    EXPECT_EQ(1U, p.GetPointCount());
    EXPECT_EQ(3U, p.GetTotalPointCount());
    EXPECT_EQ(Point2f(0, 1), p.GetStartPoint());
    EXPECT_EQ(Point2f(1, 0), p.GetEndPoint());
    EXPECT_EQ(Point2f(.2f, .2f), p.GetPoints()[0]);

    // Test error cases:

    TEST_THROW(p.RemovePoint(0), AssertException, "min_count");
    TEST_THROW(p.SetPoint(1,    Point2f(.5f, .5f)), AssertException, "index");
    TEST_THROW(p.InsertPoint(2, Point2f(.5f, .5f)), AssertException, "index");
}

