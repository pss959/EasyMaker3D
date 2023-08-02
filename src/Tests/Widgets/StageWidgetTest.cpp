#include "Place/DragInfo.h"
#include "SG/ColorMap.h"
#include "SG/Search.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Util/String.h"
#include "Widgets/StageWidget.h"

#include "Util/Read.h" // XXXX

// ----------------------------------------------------------------------------
// StageWidgetTest class.
// ----------------------------------------------------------------------------

class StageWidgetTest : public SceneTestBase {
  protected:
    StageWidgetPtr GetStageWidget();
  private:
    SG::ScenePtr scene_;  // Saves scene used to read StageWidget.
};

StageWidgetPtr StageWidgetTest::GetStageWidget() {
    // The StageWidget requires some set-up; reading the Stage.emd file and
    // adding it to a real Scene is the easiest way to take care of this.
    const std::string contents = "children: [ <\"nodes/Stage.emd\"> ]";
    const auto input =
        Util::ReplaceString(ReadDataFile("RealScene"), "#<CONTENTS>", contents);
    scene_ = ReadScene(input, true);   // True => sets up grid image.
    auto sw = SG::FindTypedNodeInScene<StageWidget>(*scene_, "Stage");
    EXPECT_NOT_NULL(sw);
    return sw;
}

// ----------------------------------------------------------------------------
// Tests.
// ----------------------------------------------------------------------------

TEST_F(StageWidgetTest, Transforms) {
    auto sw = GetStageWidget();
    EXPECT_EQ(1, sw->GetRadiusScale());
    EXPECT_EQ(Vector3f(1, 1, 1), sw->GetScale());
    EXPECT_EQ(Rotationf::Identity(), sw->GetRotation());

    sw->SetScaleAndRotation(3, Anglef::FromDegrees(30));
    EXPECT_EQ(Vector3f(3, 3, 3), sw->GetScale());
    EXPECT_EQ(BuildRotation(0, 1, 0, 30), sw->GetRotation());

    auto scaler = sw->GetStageScaleNode();
    EXPECT_NOT_NULL(scaler);
    EXPECT_EQ(Vector3f(1, 1, 1), scaler->GetScale());

    // Changing the scale should affect only the X and Z sizes of the stage
    // geometry, which is always 1 unit high.
    const auto &geom = sw->GetStageGeometryNode();
    float ht = sw->GetScale()[1] * scaler->GetScale()[1] * geom->GetScale()[1];
    EXPECT_CLOSE(1, ht);

    // Compensate for a change in radius.
    sw->SetStageRadius(100);
    EXPECT_VECS_CLOSE(Vector3f(.32f, .32f, .32f), scaler->GetScale());
    ht = sw->GetScale()[1] * scaler->GetScale()[1] * geom->GetScale()[1];
    EXPECT_CLOSE(1, ht);

    // Change the stage size.
    sw->ApplyScaleChange(1);
    EXPECT_VECS_CLOSE(Vector3f(.32f, .32f, .32f), scaler->GetScale());
    ht = sw->GetScale()[1] * scaler->GetScale()[1] * geom->GetScale()[1];
    EXPECT_CLOSE(1, ht);
}

TEST_F(StageWidgetTest, PointTarget) {
    auto sw = GetStageWidget();

    EXPECT_TRUE(sw->CanReceiveTarget());

    // Snap the PointTarget to grid points. Snapping relies only on the X and Z
    // of the SG::Hit point location and the linear precision.
    DragInfo       info;
    Point3f        pos;
    Vector3f       dir;
    Dimensionality dims;

    info.hit.point.Set(3.25f, 0, -1.9f);
    info.linear_precision = 1;
    sw->PlacePointTarget(info, pos, dir, dims);
    EXPECT_EQ(Point3f(3, 0, -2),  pos);
    EXPECT_EQ(Vector3f::AxisY(),  dir);
    EXPECT_EQ(0, dims.GetCount());

    // Finer precision
    info.hit.point.Set(3.28f, 1, -1.9f);
    info.linear_precision = .1f;
    sw->PlacePointTarget(info, pos, dir, dims);
    EXPECT_EQ(Point3f(3.3f, 0, -1.9f),  pos);
    EXPECT_EQ(Vector3f::AxisY(),  dir);
    EXPECT_EQ(0, dims.GetCount());
}

TEST_F(StageWidgetTest, EdgeTarget) {
    auto sw = GetStageWidget();

    EXPECT_TRUE(sw->CanReceiveTarget());

    // Snap the EdgeTarget to grid edges. Snapping relies only on the X and Z
    // of the SG::Hit point location and the linear precision. The length
    // should not change.
    DragInfo       info;
    float          length = 3;
    Point3f        pos0, pos1;

    info.hit.point.Set(3.25f, 0, -1.9f);
    info.linear_precision = 1;
    sw->PlaceEdgeTarget(info, length, pos0, pos1);
    EXPECT_EQ(3, length);
    EXPECT_EQ(Point3f(3, 0, -2),  pos0);
    EXPECT_EQ(Point3f(3, 3, -2),  pos1);

    // Finer precision
    info.hit.point.Set(3.28f, 1, -1.9f);
    info.linear_precision = .1f;
    sw->PlaceEdgeTarget(info, length, pos0, pos1);
    EXPECT_EQ(3, length);
    EXPECT_EQ(Point3f(3.3f, 0, -1.9f),  pos0);
    EXPECT_EQ(Point3f(3.3f, 3, -1.9f),  pos1);
}
