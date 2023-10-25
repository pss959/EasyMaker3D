#include "App/ToolBox.h"
#include "Managers/CommandManager.h"
#include "Managers/TargetManager.h"
#include "Models/BoxModel.h"
#include "Models/CylinderModel.h"
#include "Models/RootModel.h"
#include "SG/Search.h"
#include "Selection/Selection.h"
#include "Tests/Testing2.h"
#include "Tests/Tools/ToolTestBase.h"
#include "Tests/Widgets/DragTester.h"
#include "Tools/CylinderTool.h"
#include "Tools/FindTools.h"
#include "Util/Assert.h"
#include "Widgets/DraggableWidget.h"
#include "Widgets/PointTargetWidget.h"

// ----------------------------------------------------------------------------
// ToolBoxTest class.
// ----------------------------------------------------------------------------

/// \ingroup Tests
class ToolBoxTest : public ToolTestBase {
  public:
    /// Derived Tool class that calls Tool::Finish() that is used for the
    /// CompletionFunc test.
    class TestTool : public CylinderTool {
      public:
        using Tool::Finish;  // Makes it callable.
    };
    DECL_SHARED_PTR(TestTool);

    ToolBoxPtr tool_box;
    RootModelPtr     root;
    BoxModelPtr      box;
    CylinderModelPtr cyl;
    Selection        box_sel;
    Selection        cyl_sel;
    Selection        both_sel;

    /// The constructor sets up a ToolBox instance, a Model hierarchy, and
    /// Selection instances for testing attachment of Tools to a Selection.
    ToolBoxTest();

    /// Tests that the current tool is not null and is of the named type.
    void TestCurrentTool(const Str &type_name) {
        EXPECT_NOT_NULL(tool_box->GetCurrentTool());
        EXPECT_EQ(type_name, tool_box->GetCurrentTool()->GetTypeName());
    }

    /// Tests that the tool attached to the given Model is not null and is of
    /// the named type.
    void TestAttachedTool(const ModelPtr &model, const Str &type_name) {
        EXPECT_NOT_NULL(tool_box->GetAttachedTool(model));
        EXPECT_EQ(type_name, tool_box->GetAttachedTool(model)->GetTypeName());
    }
};

ToolBoxTest::ToolBoxTest() {
    const Str extra_children = R"(<"nodes/Shelf.emd">, <"nodes/Shelves.emd">,)";

    // Read the scene and set up the Models
    const auto &scene = *ReadToolScene(extra_children);
    root = SG::FindTypedNodeInScene<RootModel>(scene, "ModelRoot");
    box  = Model::CreateModel<BoxModel>("Box");
    cyl  = Model::CreateModel<CylinderModel>("Cyl");
    root->AddChildModel(box);
    root->AddChildModel(cyl);

    // Find and set up all known Tools.
    const auto tools = FindTools(scene);
    for (auto &tool: tools)
        SetUpTool(tool);

    // Now that the base class has set up the Tool::Context with all managers,
    // create the ToolBox and add all the Tools to it.
    ASSERT(context->target_manager);
    tool_box.reset(new ToolBox(*context->target_manager));
    tool_box->SetParentNode(scene.GetRootNode());
    tool_box->AddTools(tools);

    /// Set up Selection instances.
    box_sel  = Selection(SelPath(root, box));
    cyl_sel  = Selection(SelPath(root, cyl));
    both_sel = cyl_sel;
    both_sel.Add(SelPath(root, box));
}

// ----------------------------------------------------------------------------
// Tests.
// ----------------------------------------------------------------------------

TEST_F(ToolBoxTest, DefaultGeneralTool) {
    EXPECT_NULL(tool_box->GetCurrentTool());
    tool_box->SetDefaultGeneralTool("TranslationTool");
    TestCurrentTool("TranslationTool");
    EXPECT_FALSE(tool_box->IsUsingSpecializedTool());

    tool_box->ClearTools();
    EXPECT_NULL(tool_box->GetCurrentTool());
}

TEST_F(ToolBoxTest, GeneralTools) {
    EXPECT_TRUE(tool_box->CanUseGeneralTool("ScaleTool",       box_sel));
    EXPECT_TRUE(tool_box->CanUseGeneralTool("RotationTool",    cyl_sel));
    EXPECT_FALSE(tool_box->CanUseGeneralTool("ComplexityTool", box_sel));
    EXPECT_TRUE(tool_box->CanUseGeneralTool("ComplexityTool",  cyl_sel));

    tool_box->UseGeneralTool("ScaleTool", box_sel);
    TestCurrentTool("ScaleTool");
    tool_box->UseNextGeneralTool(box_sel);
    TestCurrentTool("TranslationTool");
    tool_box->UsePreviousGeneralTool(box_sel);
    TestCurrentTool("ScaleTool");

    // Test wrapping around in both directions.
    while (tool_box->GetCurrentTool()->GetTypeName() != "TranslationTool")
        tool_box->UsePreviousGeneralTool(box_sel);
    TestCurrentTool("TranslationTool");
    while (tool_box->GetCurrentTool()->GetTypeName() != "ScaleTool")
        tool_box->UseNextGeneralTool(box_sel);
    TestCurrentTool("ScaleTool");
}

TEST_F(ToolBoxTest, SpecializedTools) {
    EXPECT_FALSE(tool_box->CanUseSpecializedTool(box_sel));
    EXPECT_TRUE(tool_box->CanUseSpecializedTool(cyl_sel));

    EXPECT_NULL(tool_box->GetSpecializedToolForSelection(box_sel));
    auto tool = tool_box->GetSpecializedToolForSelection(cyl_sel);
    EXPECT_NOT_NULL(tool);
    EXPECT_EQ("CylinderTool", tool->GetTypeName());

    tool_box->UseGeneralTool("ScaleTool", box_sel);
    TestCurrentTool("ScaleTool");
    tool_box->UseSpecializedTool(box_sel);  // No effect.
    TestCurrentTool("ScaleTool");

    tool_box->DetachTools(box_sel);
    TestCurrentTool("ScaleTool");

    tool_box->UseSpecializedTool(cyl_sel);
    TestCurrentTool("CylinderTool");
    tool_box->UseSpecializedTool(cyl_sel);  // No effect.
    TestCurrentTool("CylinderTool");

    tool_box->ToggleSpecializedTool(cyl_sel);
    TestCurrentTool("ScaleTool");

    tool_box->DetachTools(cyl_sel);
    TestCurrentTool("ScaleTool");

    tool_box->ToggleSpecializedTool(box_sel);
    TestCurrentTool("ScaleTool");
}

TEST_F(ToolBoxTest, AttachDetach) {
    tool_box->SetDefaultGeneralTool("TranslationTool");
    TestCurrentTool("TranslationTool");

    EXPECT_NULL(tool_box->GetAttachedTool(box));
    EXPECT_NULL(tool_box->GetAttachedTool(cyl));

    tool_box->AttachToSelection(Selection());  // No effect.
    TestCurrentTool("TranslationTool");
    EXPECT_NULL(tool_box->GetAttachedTool(box));
    EXPECT_NULL(tool_box->GetAttachedTool(cyl));

    tool_box->AttachToSelection(box_sel);
    TestCurrentTool("TranslationTool");
    TestAttachedTool(box, "TranslationTool");
    EXPECT_NULL(tool_box->GetAttachedTool(cyl));
    tool_box->DetachTools(box_sel);
    TestCurrentTool("TranslationTool");
    EXPECT_NULL(tool_box->GetAttachedTool(box));
    EXPECT_NULL(tool_box->GetAttachedTool(cyl));

    tool_box->AttachToSelection(cyl_sel);
    TestCurrentTool("TranslationTool");
    EXPECT_NULL(tool_box->GetAttachedTool(box));
    TestAttachedTool(cyl, "TranslationTool");
    tool_box->DetachTools(cyl_sel);
    TestCurrentTool("TranslationTool");
    EXPECT_NULL(tool_box->GetAttachedTool(box));
    EXPECT_NULL(tool_box->GetAttachedTool(cyl));

    // Attach while a Specialized Tool is in effect.
    tool_box->UseSpecializedTool(cyl_sel);
    TestCurrentTool("CylinderTool");
    EXPECT_NULL(tool_box->GetAttachedTool(box));
    TestAttachedTool(cyl, "CylinderTool");
    tool_box->DetachTools(cyl_sel);
    TestCurrentTool("CylinderTool");
    EXPECT_NULL(tool_box->GetAttachedTool(box));
    EXPECT_NULL(tool_box->GetAttachedTool(cyl));
    tool_box->AttachToSelection(cyl_sel);
    TestCurrentTool("CylinderTool");
    EXPECT_NULL(tool_box->GetAttachedTool(box));
    TestAttachedTool(cyl, "CylinderTool");
    tool_box->DetachTools(cyl_sel);

    // Cannot attach a specialized tool to both, so should use a general tool.
    tool_box->AttachToSelection(both_sel);
    TestCurrentTool("TranslationTool");
    TestAttachedTool(cyl, "TranslationTool");
    TestAttachedTool(box, "PassiveTool");
    tool_box->ReattachTools();
    TestAttachedTool(cyl, "TranslationTool");
    TestAttachedTool(box, "PassiveTool");
    tool_box->DetachTools(both_sel);
    TestCurrentTool("TranslationTool");
    EXPECT_NULL(tool_box->GetAttachedTool(box));
    EXPECT_NULL(tool_box->GetAttachedTool(cyl));

    // Cannot attach ComplexityTool to a BoxModel, so skip it.
    tool_box->UseGeneralTool("ComplexityTool", cyl_sel);
    TestCurrentTool("ComplexityTool");
    EXPECT_NULL(tool_box->GetAttachedTool(box));
    TestAttachedTool(cyl, "ComplexityTool");
    tool_box->DetachTools(cyl_sel);
    tool_box->AttachToSelection(box_sel);
    TestCurrentTool("RotationTool");
    TestAttachedTool(box, "RotationTool");
    EXPECT_NULL(tool_box->GetAttachedTool(cyl));
}

TEST_F(ToolBoxTest, Grip) {
    Grippable::GripInfo info;
    info.guide_direction.Set(1, 0, 0);

    // Has to be a current Tool for UpdateGripInfo().
    TEST_ASSERT(tool_box->UpdateGripInfo(info), "GetCurrentTool");

    tool_box->SetDefaultGeneralTool("ScaleTool");
    EXPECT_NULL(tool_box->GetGrippableNode());
    tool_box->AttachToSelection(box_sel);
    auto node = tool_box->GetGrippableNode();
    EXPECT_NOT_NULL(node);
    EXPECT_EQ("ScaleTool", node->GetTypeName());
    TestCurrentTool("ScaleTool");

    // Should get a 1D scaler in the ScaleTool.
    tool_box->UpdateGripInfo(info);
    EXPECT_ENUM_EQ(GripGuideType::kBasic, info.guide_type);
    EXPECT_NOT_NULL(info.widget);
    EXPECT_EQ("Slider1DWidget", info.widget->GetTypeName());
}

TEST_F(ToolBoxTest, Drag) {
    AddDummyCommandFunction("TranslateCommand");

    tool_box->UseGeneralTool("TranslationTool", both_sel);
    tool_box->AttachToSelection(both_sel);
    TestCurrentTool("TranslationTool");
    TestAttachedTool(cyl, "TranslationTool");
    TestAttachedTool(box, "PassiveTool");

    // Use these to make sure that the PassiveTool attached to the BoxModel is
    // disabled during the drag. Note that this relies on these callbacks being
    // added/invoked after the callbacks in the Tool class.
    auto test_start = [&](Tool &tool){
        auto passive = tool_box->GetAttachedTool(box);
        EXPECT_NOT_NULL(passive);
        EXPECT_FALSE(passive->IsEnabled());
    };
    auto test_end = [&](Tool &tool){
        auto passive = tool_box->GetAttachedTool(box);
        EXPECT_NOT_NULL(passive);
        EXPECT_TRUE(passive->IsEnabled());
    };

    auto tool = tool_box->GetCurrentTool();

    tool->GetDragStarted().AddObserver("key", test_start);
    tool->GetDragEnded().AddObserver("key", test_end);

    auto xs = SG::FindTypedNodeUnderNode<DraggableWidget>(*tool, "XSlider");
    DragTester dt(xs);
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(1, 0, 0));
}

TEST_F(ToolBoxTest, ReattachOnModelChange) {
    tool_box->UseGeneralTool("TranslationTool", cyl_sel);
    cyl->SetTopRadius(5);
    TestCurrentTool("TranslationTool");
}

TEST_F(ToolBoxTest, DisableOnTargetActivation) {
    // Activating the PointTargetWidget or EdgeTargetWidget should disable the
    // current tool.
    tool_box->UseGeneralTool("TranslationTool", box_sel);
    tool_box->AttachToSelection(box_sel);
    TestCurrentTool("TranslationTool");

    auto tool = tool_box->GetAttachedTool(box);
    EXPECT_NOT_NULL(tool);
    EXPECT_TRUE(tool->IsEnabled());

    // Use these to make sure that the Tool attached to the BoxModel is
    // disabled during the drag of the PointTargetWidget.
    auto test_enabled = [&](const DragInfo *info, bool is_start){
        if (info && ! is_start) {  // During drag.
            EXPECT_FALSE(tool->IsEnabled());
        }
    };
    auto ptw = SG::FindTypedNodeInScene<PointTargetWidget>(
        *GetScene(), "PointTargetWidget");
    ptw->SetStageToWorldMatrix(Matrix4f::Identity());
    ptw->GetDragged().AddObserver("key", test_enabled);

    // The TargetManager requires a path to the stage for this to work.
    context->target_manager->SetPathToStage(
        SG::NodePath(GetScene()->GetRootNode()));

    DragTester dt(ptw);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(0, 1, 2), Point3f(6, 1.5f, 2));
    EXPECT_TRUE(tool->IsEnabled());
}

TEST_F(ToolBoxTest, CompletionFunc) {
    // Of the real Tool classes, only ImportTool calls Tool::Finish(), which
    // invokes the completion function. Use the TestTool class, which also
    // calls it.
    TestToolPtr tt(new TestTool);
    tool_box->AddTools(std::vector<ToolPtr>{ tt });

    tool_box->SetDefaultGeneralTool("TranslationTool");

    // Attach the CylinderTool to the CylinderModel.
    tool_box->UseSpecializedTool(cyl_sel);
    TestCurrentTool("CylinderTool");

    // Calling this should invoke the ToolBox's completion function, which
    // should switch back to a general Tool.
    tt->Finish();
    TestCurrentTool("TranslationTool");
}
