#include "Math/Taper.h"
#include "Models/BoxModel.h"
#include "Models/TaperedModel.h"
#include "Tests/Testing.h"
#include "Tests/SceneTestBase.h"
#include "Util/Tuning.h"

class TaperedModelTest : public SceneTestBase {
};

TEST_F(TaperedModelTest, TaperProfile) {
    // Tests valid and invalid Taper Profiles.
    Profile prof;

    // Default Profile is not valid.
    EXPECT_FALSE(Taper::IsValidProfile(prof));

    const Point2f p0(0, 1);
    const Point2f p1(1, 0);

    const Point2f pa(.7f, .7f);
    const Point2f pb(.5f, .5f);
    const Point2f pc(.3f, .3f);

    const Point2f pbad0(.3f, 1.1f);
    const Point2f pbad1(.3f, -.1f);

    const Point2f pb2 = pb + Vector2f(0, .5f * TK::kMinTaperProfileYDistance);

    // Wrong type.
    prof = Profile::CreateFixedProfile(p0, p1, 2, Profile::PointVec());
    EXPECT_FALSE(Taper::IsValidProfile(prof));

    const auto buildprof = [&](size_t min, const Profile::PointVec &pts){
        return Profile(Profile::Type::kOpen, min, pts);
    };

    // Not enough points.
    prof = buildprof(1, Profile::PointVec{p0});
    EXPECT_FALSE(Taper::IsValidProfile(prof));

    // Valid 2-point profile.
    prof = buildprof(2, Profile::PointVec{p0, p1});
    EXPECT_TRUE(Taper::IsValidProfile(prof));

    // Wrong starting endpoint Y.
    prof = buildprof(2, Profile::PointVec{Point2f(0, .9f), p1});
    EXPECT_FALSE(Taper::IsValidProfile(prof));

    // Wrong ending endpoint Y.
    prof = buildprof(2, Profile::PointVec{p0, Point2f(0, .01f)});
    EXPECT_FALSE(Taper::IsValidProfile(prof));

    // Valid 3-point profile.
    prof = buildprof(2, Profile::PointVec{p0, pa, p1});
    EXPECT_TRUE(Taper::IsValidProfile(prof));

    // Valid 4-point profile.
    prof = buildprof(2, Profile::PointVec{p0, pa, pb, p1});
    EXPECT_TRUE(Taper::IsValidProfile(prof));

    // Valid 5-point profile.
    prof = buildprof(2, Profile::PointVec{p0, pa, pb, pc, p1});
    EXPECT_TRUE(Taper::IsValidProfile(prof));

    // Out of range 3-point profiles.
    prof = buildprof(2, Profile::PointVec{p0, pbad0, p1});
    EXPECT_FALSE(Taper::IsValidProfile(prof));
    prof = buildprof(2, Profile::PointVec{p0, pbad1, p1});
    EXPECT_FALSE(Taper::IsValidProfile(prof));

    // Out of order 4-point profile.
    prof = buildprof(2, Profile::PointVec{p0, pb, pa, p1});
    EXPECT_FALSE(Taper::IsValidProfile(prof));

    // Non-increasing 4-point profile.
    prof = buildprof(2, Profile::PointVec{p0, pb, pb, p1});
    EXPECT_FALSE(Taper::IsValidProfile(prof));

    // 4-point profile not increasing enough.
    prof = buildprof(2, Profile::PointVec{p0, pb2, pb, p1});
    EXPECT_FALSE(Taper::IsValidProfile(prof));
}

// XXXX Add actual TaperedModel tests. Check slicing, etc...
