#include "Place/DragInfo.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Widgets/StageWidget.h"

/// \ingroup Tests
class StageWidgetTest : public SceneTestBase {
  protected:
    StageWidgetPtr GetStageWidget() {
        return ReadRealNode<StageWidget>(
            R"(children: [<"nodes/Stage.emd">])", "Stage");
    }
};

TEST_F(StageWidgetTest, Transforms) {
    auto sw = GetStageWidget();
    const auto &geom = sw->GetStageGeometryNode();

    EXPECT_EQ(32, sw->GetRadiusScale());
    EXPECT_EQ(Vector3f(1, 1, 1), sw->GetScale());
    EXPECT_EQ(Rotationf::Identity(), sw->GetRotation());

    // The top of the stage should always be at Y=0.
    const auto get_y_top = [&](){
        return geom->GetTranslation()[1] +
            geom->GetScale()[1] * geom->GetBounds().GetMaxPoint()[1];
    };
    EXPECT_CLOSE(0, get_y_top());

    sw->SetScaleAndRotation(3, Anglef::FromDegrees(30));
    EXPECT_EQ(Vector3f(3, 3, 3), sw->GetScale());
    EXPECT_EQ(BuildRotation(0, 1, 0, 30), sw->GetRotation());

    auto scaler = sw->GetStageScaleNode();
    EXPECT_NOT_NULL(scaler);
    EXPECT_EQ(Vector3f(32, 32, 32), scaler->GetScale());
    EXPECT_CLOSE(0, get_y_top());

    // Compensate for a change in radius.
    sw->SetStageRadius(100);
    EXPECT_VECS_CLOSE(Vector3f(.32f, .32f, .32f), scaler->GetScale());
    EXPECT_CLOSE(0, get_y_top());

    // Change the stage size.
    EXPECT_TRUE(sw->ProcessValuator(1));
    EXPECT_VECS_CLOSE(Vector3f(.32f, .32f, .32f), scaler->GetScale());
    EXPECT_CLOSE(0, get_y_top());
}

TEST_F(StageWidgetTest, PointTarget) {
    auto sw = GetStageWidget();
    EXPECT_FALSE(sw->CanTargetBounds());

    // Set the radius so that the math is consistent.
    sw->SetStageRadius(20);

    // Snap the PointTarget to grid points. Snapping relies only on the X and Z
    // of the SG::Hit point location and the linear precision.
    DragInfo       info;
    Point3f        pos;
    Vector3f       dir;
    Dimensionality dims;

    info.hit.point.Set(3.25f, 0, -1.9f);
    info.linear_precision = 1;
    sw->PlacePointTarget(info, pos, dir, dims);
    EXPECT_EQ(Point3f(2, 0, -1),  pos);
    EXPECT_EQ(Vector3f::AxisY(),  dir);
    EXPECT_EQ(0, dims.GetCount());

    // Finer precision
    info.hit.point.Set(3.28f, 1, -1.9f);
    info.linear_precision = .1f;
    sw->PlacePointTarget(info, pos, dir, dims);
    EXPECT_PTS_CLOSE(Point3f(2.1f, 0, -1.2f),  pos);
    EXPECT_EQ(Vector3f::AxisY(),  dir);
    EXPECT_EQ(0, dims.GetCount());
}

TEST_F(StageWidgetTest, EdgeTarget) {
    auto sw = GetStageWidget();

    // Set the radius so that the math is consistent.
    sw->SetStageRadius(20);

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
    EXPECT_EQ(Point3f(2, 0, -1),  pos0);
    EXPECT_EQ(Point3f(2, 3, -1),  pos1);

    // Finer precision
    info.hit.point.Set(3.28f, 1, -1.9f);
    info.linear_precision = .1f;
    sw->PlaceEdgeTarget(info, length, pos0, pos1);
    EXPECT_EQ(3, length);
    EXPECT_PTS_CLOSE(Point3f(2.1f, 0, -1.2f),  pos0);
    EXPECT_PTS_CLOSE(Point3f(2.1f, 3, -1.2f),  pos1);
}
