#include "Commands/TranslateCommand.h"
#include "Models/BoxModel.h"
#include "Models/RootModel.h"
#include "SG/Search.h"
#include "Tools/TranslationTool.h"
#include "Tests/Tools/ToolTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Widgets/DraggableWidget.h"

DECL_SHARED_PTR(TranslationTool);

/// \ingroup Tests
class TranslationToolTest : public ToolTestBase {
  protected:
    TranslationToolPtr tool;
    ModelPtr        model;   ///< Model the Tool is attached to.

    /// The constructor sets up a TranslationTool attached to a selected
    /// BoxModel.
    TranslationToolTest();

    /// The destructor detaches the TranslationTool.
    ~TranslationToolTest() { tool->DetachFromSelection(); }

    /// Returns the named translation DraggableWidget.
    DraggableWidgetPtr FindSlider(const Str &name) {
        return SG::FindTypedNodeUnderNode<DraggableWidget>(*tool, name);
    }
};

TranslationToolTest::TranslationToolTest() {
    // Initialize the TranslationTool.
    tool = InitTool<TranslationTool>("TranslationTool");
    EXPECT_FALSE(tool->IsSpecialized());

    // Create and add a BoxModel.
    model = Model::CreateModel<BoxModel>("Box");
    context->root_model->AddChildModel(model);

    // Attach the TranslationTool to the BoxModel.
    Selection sel(SelPath(context->root_model, model));
    EXPECT_TRUE(tool->CanBeUsedFor(sel));
    tool->AttachToSelection(sel, 0);

    EXPECT_NOT_NULL(tool->GetModelAttachedTo());

    AddDummyCommandFunction("TranslateCommand");
}

TEST_F(TranslationToolTest, UpdateGripInfo) {
    Grippable::GripInfo info;

    info.guide_direction.Set(1, 0, 0);
    tool->UpdateGripInfo(info);
    EXPECT_ENUM_EQ(GripGuideType::kBasic,  info.guide_type);
    EXPECT_PTS_CLOSE(Point3f(-1.2f, 0, 0), info.target_point);
    EXPECT_EQ(FindSlider("XSlider"),       info.widget);
    EXPECT_EQ("#dd595bff",                 info.color.ToHexString());

    info.guide_direction.Set(0, -1, 0);
    tool->UpdateGripInfo(info);
    EXPECT_ENUM_EQ(GripGuideType::kBasic,  info.guide_type);
    EXPECT_PTS_CLOSE(Point3f(0, 1.2f, 0),  info.target_point);
    EXPECT_EQ(FindSlider("YSlider"),       info.widget);
    EXPECT_EQ("#4b80e2ff",                 info.color.ToHexString());

    info.guide_direction.Set(0, 0, 1);
    tool->UpdateGripInfo(info);
    EXPECT_ENUM_EQ(GripGuideType::kBasic,  info.guide_type);
    EXPECT_PTS_CLOSE(Point3f(0, 0, -1.2f), info.target_point);
    EXPECT_EQ(FindSlider("ZSlider"),       info.widget);
    EXPECT_EQ("#4bc44bff",                 info.color.ToHexString());

    // Otherwise, no widget is used.
    info.guide_direction = Normalized(1, 1, -1);
    tool->UpdateGripInfo(info);
    EXPECT_NULL(info.widget);
}

TEST_F(TranslationToolTest, Translate) {
    CheckNoCommands();

    DragTester dt(FindSlider("XSlider"));
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(1, 0, 0));

    const auto &cmd = CheckOneCommand<TranslateCommand>();
    EXPECT_EQ(StrVec{ "Box" },    cmd.GetModelNames());
    EXPECT_EQ(Vector3f(1, 0, 0),  cmd.GetTranslation());
}

TEST_F(TranslationToolTest, SnapToTarget) {
    CheckNoCommands();

    SetPointTarget(Point3f(2, 0, 0), Vector3f(0, 1, 0));
    SetIsAxisAligned(true);

    DragTester dt(FindSlider("XSlider"));
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(2.05f, 0, 0));

    const auto &cmd = CheckOneCommand<TranslateCommand>();
    EXPECT_EQ(StrVec{ "Box" },    cmd.GetModelNames());
    EXPECT_EQ(Vector3f(2, 0, 0),  cmd.GetTranslation());
}
