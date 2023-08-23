#include <ion/math/vectorutils.h>

#include "Math/Profile.h"
#include "Panes/ProfilePane.h"
#include "Panes/TaperProfilePane.h"
#include "Place/ClickInfo.h"
#include "Place/TouchInfo.h"
#include "Tests/Panes/PaneTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Util/Tuning.h"
#include "Widgets/DraggableWidget.h"

/// \ingroup Tests
class ProfilePaneTest : public PaneTestBase {
  protected:
    /// \name Hover/drag data.
    /// These variables are set by SetUpHoverAndDrag() for testing hovering and
    /// dragging.
    ///@{
    DraggableWidgetPtr aw;   ///< Area widget.
    DraggableWidgetPtr npw;  ///< New-point widget.
    SG::NodePtr        del;  ///< Delete spot.
    ///@}

    ProfilePanePtr GetProfilePane(const Str &contents = "") {
        return ReadRealPane<ProfilePane>("ProfilePane", contents);
    }

    /// Sets up for a hover or drag test. If \p set_profile is true, this also
    /// installs a profile with an interior point. Profile points are at (0,1),
    /// (.5,.5), and (1,0) in profile coordinates, which are (-.5,.5), (0,0),
    /// and (.5,-.5) in object/world coordinates. This also access the #aw and
    /// #npw member variables.
    void SetUpHoverAndDrag(const ProfilePanePtr &pp, bool set_profile = true) {
        aw  = SG::FindTypedNodeUnderNode<DraggableWidget>(*pp, "AreaWidget");
        npw = SG::FindTypedNodeUnderNode<DraggableWidget>(*pp, "NewPoint");
        del = SG::FindNodeUnderNode(*pp, "DeleteSpot");
        if (set_profile) {
            pp->SetProfile(Profile::CreateFixedProfile(
                               Point2f(0, 1), Point2f(1, 0), 3,
                               Profile::PointVec{Point2f(.5f, .5f)}));
        }
    }
};

TEST_F(ProfilePaneTest, Defaults) {
    auto pp = GetProfilePane();
    EXPECT_EQ(pp.get(),  pp->GetInteractor());
    EXPECT_EQ(Profile::CreateFixedProfile(Point2f(0, 1), Point2f(1, 0), 2,
                                          Profile::PointVec()),
              pp->GetProfile());
    // Cannot activate or focus on a ProfilePane.
    EXPECT_NULL(pp->GetActivationWidget());
    EXPECT_NULL(pp->GetFocusBorder());
}

TEST_F(ProfilePaneTest, SetProfile) {
    auto pp = GetProfilePane();

    const Profile prof = Profile::CreateFixedProfile(
        Point2f(0, 0), Point2f(0, 1), 3, Profile::PointVec{Point2f(.8f, .5f)});
    pp->SetProfile(prof);
    EXPECT_EQ(prof, pp->GetProfile());
}

TEST_F(ProfilePaneTest, MouseHover) {
    // Tests that hovering the mouse near a profile line creates a new widget
    // for creating and moving a point.

    auto pp = GetProfilePane();
    SetUpHoverAndDrag(pp);

    EXPECT_FALSE(npw->IsEnabled());

    // Hover away from the profile line. The new-point widget should remain
    // disabled.
    aw->UpdateHoverPoint(Point3f(.4f, .3f, 0));
    EXPECT_FALSE(npw->IsEnabled());

    // Hover near the middle profile point. The new-point widget should remain
    // disabled.
    aw->UpdateHoverPoint(Point3f(0, 0, 0));
    EXPECT_FALSE(npw->IsEnabled());

    // Hover near the profile line but away from any point. This should enable
    // the new-point widget.
    aw->UpdateHoverPoint(Point3f(-.25f, .25f, 0));
    EXPECT_TRUE(npw->IsEnabled());
    EXPECT_EQ(Vector3f(-.25f, .25f, TK::kPaneZOffset), npw->GetTranslation());
}

TEST_F(ProfilePaneTest, GripHover) {
    // Tests the GetGripWidget() function used for grip-hovering.
    auto pp = GetProfilePane();
    SetUpHoverAndDrag(pp);

    EXPECT_FALSE(npw->IsEnabled());

    // Grip-hover near the middle (movable point).
    auto gw = pp->GetGripWidget(Point2f(.5f, .5f));
    EXPECT_NOT_NULL(gw);
    EXPECT_EQ("MovablePoint_1", gw->GetName());
    EXPECT_EQ(Vector3f(0, 0, 0), gw->GetTranslation());
    EXPECT_FALSE(npw->IsEnabled());

    // Hover near the midpoint of the top segment. Should show a new point.
    gw = pp->GetGripWidget(Point2f(.25f, .75f));
    EXPECT_EQ(npw, gw);
    EXPECT_TRUE(npw->IsEnabled());
    EXPECT_EQ(Vector3f(-.25f, .25f, TK::kPaneZOffset), npw->GetTranslation());

    // Hover back near the middle point.
    gw = pp->GetGripWidget(Point2f(.46f, .52f));
    EXPECT_NOT_NULL(gw);
    EXPECT_EQ("MovablePoint_1", gw->GetName());
    EXPECT_EQ(Vector3f(0, 0, 0), gw->GetTranslation());
    EXPECT_FALSE(npw->IsEnabled());

    // Hover near the midpoint of the bottom segment.
    gw = pp->GetGripWidget(Point2f(.8, .2f));
    EXPECT_EQ(npw, gw);
    EXPECT_TRUE(npw->IsEnabled());
    EXPECT_EQ(Vector3f(.25f, -.25f, TK::kPaneZOffset), npw->GetTranslation());
}

TEST_F(ProfilePaneTest, Touch) {
    auto pp = GetProfilePane();
    SetUpHoverAndDrag(pp);

    EXPECT_FALSE(npw->IsEnabled());

    TouchInfo info;
    info.radius = .01f;
    info.root_node = pp;

    // Set the position to the middle (movable point). This should return it.
    float dist = 1000;
    info.position.Set(0, 0, 0);
    auto iw = pp->GetTouchedWidget(info, dist);
    EXPECT_NOT_NULL(iw);
    EXPECT_EQ("MovablePoint_1", iw->GetName());
    EXPECT_EQ(Vector3f(0, 0, 0), iw->GetTranslation());
    EXPECT_FALSE(npw->IsEnabled());

    // Set the position near the midpoint of the top segment. Should show a new
    // point.
    dist = 1000;
    info.position.Set(-.25f, .25f, 0);
    iw = pp->GetTouchedWidget(info, dist);
    EXPECT_EQ(npw, iw);
    EXPECT_TRUE(npw->IsEnabled());
    EXPECT_EQ(Vector3f(-.25f, .25f, TK::kPaneZOffset), npw->GetTranslation());

    // Have to do this so the next tests work.
    npw->SetEnabled(false);

    // Miss everything.
    dist = 1000;
    info.position.Set(-.4f, -.2f, 0);
    iw = pp->GetTouchedWidget(info, dist);
    EXPECT_NULL(iw);

    // Repeat with the midpoint of the bottom segment.
    dist = 1000;
    info.position.Set(.25f, -.25f, 0);
    iw = pp->GetTouchedWidget(info, dist);
    EXPECT_EQ(npw, iw);
    EXPECT_TRUE(npw->IsEnabled());
    EXPECT_EQ(Vector3f(.25f, -.25f, TK::kPaneZOffset), npw->GetTranslation());
}

TEST_F(ProfilePaneTest, Click) {
    auto pp = GetProfilePane();
    SetUpHoverAndDrag(pp);

    EXPECT_FALSE(npw->IsEnabled());

    // Hover to enable the new-point widget.
    aw->UpdateHoverPoint(Point3f(-.25f, .25f, 0));
    EXPECT_TRUE(npw->IsEnabled());

    // Click on the new-point widget. This should create a new profile point.
    ClickInfo info;  // Contents do not matter.
    npw->Click(info);
    EXPECT_EQ(4U, pp->GetProfile().GetPointCount());
    EXPECT_EQ(Point2f(.25f, .75f), pp->GetProfile().GetPoints()[1]);
}

TEST_F(ProfilePaneTest, Drag) {
    auto pp = GetProfilePane();
    SetUpHoverAndDrag(pp);

    size_t    act_count = 0;
    size_t  deact_count = 0;
    size_t change_count = 0;
    Point3f delete_spot(0, 0, 0);
    pp->GetActivation().AddObserver("key", [&](bool is_act){
        if (is_act) {
            ++act_count;
            delete_spot = Point3f(del->GetTranslation());
        }
        else {
            ++deact_count;
        }
    });
    pp->GetProfileChanged().AddObserver("key", [&](){ ++change_count; });

    // Hover near the midpoint of the top segment to show the new-point widget.
    aw->UpdateHoverPoint(Point3f(-.25f, .25f, 0));
    EXPECT_TRUE(npw->IsEnabled());
    EXPECT_EQ(0U,    act_count);
    EXPECT_EQ(0U,  deact_count);
    EXPECT_EQ(0U, change_count);
    EXPECT_EQ(3U, pp->GetProfile().GetPointCount());

    // Drag the midpoint to create a new point.
    {
        DragTester dt(npw);
        dt.SetRayDirection(-Vector3f::AxisZ());
        dt.ApplyMouseDrag(Point3f(-.25f, .25f, 0), Point3f(-.1f, .25f, 0));
    }
    EXPECT_EQ(1U,    act_count);
    EXPECT_EQ(1U,  deact_count);
    EXPECT_EQ(1U, change_count);
    EXPECT_EQ(4U, pp->GetProfile().GetPointCount());
    EXPECT_EQ(Point2f(.4f, .75f),   pp->GetProfile().GetPoints()[1]);

    auto mp =
        SG::FindTypedNodeUnderNode<DraggableWidget>(*pp, "MovablePoint_1");
    EXPECT_VECS_CLOSE(Vector3f(-.1f, .25f, 0), mp->GetTranslation());
    {
        // Zero drag of the new movable point to set the delete spot position.
        DragTester dt(mp);
        dt.ApplyMouseDrag(Point3f(-.1f, .25f, 0), Point3f(-.1f, .25f, 0));
        EXPECT_NE(Point3f(0, 0, 0), delete_spot);
        EXPECT_EQ(2U,    act_count);
        EXPECT_EQ(2U,  deact_count);
        EXPECT_EQ(2U, change_count);
        EXPECT_EQ(4U, pp->GetProfile().GetPointCount());

        // Drag the new point to the delete spot.
        dt.ApplyMouseDrag(Point3f(-.1f, .25f, 0), delete_spot);
        EXPECT_EQ(3U,    act_count);
        EXPECT_EQ(3U,  deact_count);
        EXPECT_EQ(4U, change_count);
        EXPECT_EQ(3U, pp->GetProfile().GetPointCount());
    }
}

TEST_F(ProfilePaneTest, DragWithPrecision) {
    auto pp = GetProfilePane();
    SetUpHoverAndDrag(pp);
    pp->SetPointPrecision(Vector2f(.2f, .2f));

    // Drag the movable point with precision.
    auto mp =
        SG::FindTypedNodeUnderNode<DraggableWidget>(*pp, "MovablePoint_1");
    EXPECT_EQ(Vector3f(0, 0, 0), mp->GetTranslation());

    DragTester dt(mp);
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(.235f, -.42f, 0));

    // Precision is applied in profile coordinates; this is the equivalent.
    EXPECT_EQ(Vector3f(.3f, -.5f, 0), mp->GetTranslation());
}

TEST_F(ProfilePaneTest, DragClosedWithSnapping) {
    // Tests closed profile and 2D snapping.

    auto pp = GetProfilePane();

    // Create a closed profile with a square.
    const Profile::PointVec pts{
        Point2f(.25f, .25f),  // MovablePoint_0: bottom left
        Point2f(.75f, .25f),  // MovablePoint_1: bottom right
        Point2f(.75f, .75f),  // MovablePoint_2: top right
        Point2f(.25f, .75f),  // MovablePoint_3: top left
    };
    const Profile prof(Profile::Type::kClosed, 3, pts);
    EXPECT_TRUE(prof.IsValid());
    pp->SetProfile(prof);

    // Drag the bottom-right corner up and a little left to snap to the right
    // edge of the square at the end point of a segment.
    {
        auto mp =
            SG::FindTypedNodeUnderNode<DraggableWidget>(*pp, "MovablePoint_1");
        EXPECT_VECS_CLOSE(Vector3f(.25f, -.25f, 0), mp->GetTranslation());
        DragTester dt(mp);
        dt.SetIsModifiedMode(true);  // Turns on snapping.
        dt.ApplyMouseDrag(Point3f(.25f, -.25f, 0), Point3f(.21f, -.1f, 0));
        EXPECT_VECS_CLOSE(Vector3f(.25f, -.1f, 0), mp->GetTranslation());
    }

    // Reset and drag to snap to starting point of a segment.
    pp->SetProfile(prof);
    {
        auto mp =
            SG::FindTypedNodeUnderNode<DraggableWidget>(*pp, "MovablePoint_0");
        EXPECT_VECS_CLOSE(Vector3f(-.25f, -.25f, 0), mp->GetTranslation());
        DragTester dt(mp);
        dt.SetIsModifiedMode(true);  // Turns on snapping.
        dt.ApplyMouseDrag(Point3f(-.25f, -.25f, 0), Point3f(-.21f, -.1f, 0));
        EXPECT_VECS_CLOSE(Vector3f(-.25f, -.1f, 0), mp->GetTranslation());
    }

    // Reset and drag to snap to both points of a segment.
    pp->SetProfile(prof);
    {
        auto mp =
            SG::FindTypedNodeUnderNode<DraggableWidget>(*pp, "MovablePoint_0");
        EXPECT_VECS_CLOSE(Vector3f(-.25f, -.25f, 0), mp->GetTranslation());
        DragTester dt(mp);
        dt.SetIsModifiedMode(true);  // Turns on snapping.
        dt.ApplyMouseDrag(Point3f(-.25f, -.25f, 0), Point3f(-.01f, -.01f, 0));
        EXPECT_VECS_CLOSE(Vector3f(0, 0, 0), mp->GetTranslation());
    }

    // Reset and drag to test not snapping.
    pp->SetProfile(prof);
    {
        auto mp =
            SG::FindTypedNodeUnderNode<DraggableWidget>(*pp, "MovablePoint_0");
        EXPECT_VECS_CLOSE(Vector3f(-.25f, -.25f, 0), mp->GetTranslation());
        DragTester dt(mp);
        dt.SetIsModifiedMode(true);  // Turns on snapping.
        dt.ApplyMouseDrag(Point3f(-.25f, -.25f, 0), Point3f(-.1f, -.1f, 0));
        EXPECT_VECS_CLOSE(Vector3f(-.1f, -.1f, 0), mp->GetTranslation());
    }

    // Reset and test hovering near the segment of the closed Profile that
    // connects the last point to the first.
    pp->SetProfile(prof);
    SetUpHoverAndDrag(pp, false);
    aw->UpdateHoverPoint(Point3f(-.25f, .01f, 0));
    EXPECT_TRUE(npw->IsEnabled());
    EXPECT_VECS_CLOSE(Vector3f(-.25f, .01f, TK::kPaneZOffset),
                      npw->GetTranslation());
}

TEST_F(ProfilePaneTest, SetLayoutSize) {
    auto pp = GetProfilePane();

    // Create a movable point to make sure it gets resized.
    SetUpHoverAndDrag(pp);

    auto mp =
        SG::FindTypedNodeUnderNode<DraggableWidget>(*pp, "MovablePoint_1");

    pp->SetLayoutSize(Vector2f(200, 200));
    const auto scale = mp->GetScale();

    // The widget should have been resized in X and Y to compensate for the
    // size change.
    pp->SetLayoutSize(Vector2f(400, 400));
    EXPECT_EQ(Vector3f(.5f * scale[0], .5f * scale[1], 1), mp->GetScale());
}

TEST_F(ProfilePaneTest, TaperProfilePane) {
    auto tpp = ReadRealPane<TaperProfilePane>("TaperProfilePane", "");
    const Profile prof(Profile::Type::kOpen, 2,
                       Profile::PointVec{Point2f(0, 1), Point2f(1, 0)});
    EXPECT_TRUE(prof.IsValid());
    tpp->SetProfile(prof);
    SetUpHoverAndDrag(tpp, false);

    // Drag start point.
    {
        auto mp =
            SG::FindTypedNodeUnderNode<DraggableWidget>(*tpp, "MovablePoint_0");
        DragTester dt(mp);
        dt.SetRayDirection(-Vector3f::AxisZ());
        dt.ApplyMouseDrag(Point3f(-.5f, .5f, 0), Point3f(-.4f, .5f, 0));
        EXPECT_PTS_CLOSE2(Point2f(.1f, 1), tpp->GetProfile().GetPoints()[0]);
    }

    // Hover and click to create the middle point.
    {
        aw->UpdateHoverPoint(Point3f(0, 0, 0));
        EXPECT_TRUE(npw->IsEnabled());
        ClickInfo info;  // Contents do not matter.
        npw->Click(info);
        EXPECT_EQ(3U, tpp->GetProfile().GetPointCount());
        EXPECT_PTS_CLOSE2(Point2f(.5f, .5f), tpp->GetProfile().GetPoints()[1]);
    }

    // Drag end point.
    {
        auto mp =
            SG::FindTypedNodeUnderNode<DraggableWidget>(*tpp, "MovablePoint_2");
        DragTester dt(mp);
        dt.SetRayDirection(-Vector3f::AxisZ());
        dt.ApplyMouseDrag(Point3f(.5f, -.5f, 0), Point3f(.4f, -.5f, 0));
        EXPECT_PTS_CLOSE2(Point2f(.9f, 0), tpp->GetProfile().GetPoints()[2]);
    }
}
