#include "App/ToolBox.h"
#include "Managers/CommandManager.h"
#include "Managers/TargetManager.h"
#include "Models/BoxModel.h"
#include "Models/CylinderModel.h"
#include "Models/RootModel.h"
#include "SG/Search.h"
#include "Selection/Selection.h"
#include "Tests/TempFile.h"
#include "Tests/Tools/ToolTestBase.h"
#include "Tools/FindTools.h"

// ----------------------------------------------------------------------------
// ToolBoxTest class.
// ----------------------------------------------------------------------------

/// \ingroup Tests
class ToolBoxTest : public ToolTestBase {
  public:
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
    // Create the ToolBox.
    CommandManagerPtr command_manager(new CommandManager);
    TargetManagerPtr  target_manager(new TargetManager(command_manager));
    tool_box.reset(new ToolBox(*target_manager));

    const Str extra_children = R"(<"nodes/Shelf.emd">, <"nodes/Shelves.emd">,)";

    // Read the scene and set up the Models
    const auto &scene = *ReadToolScene(extra_children);
    root = SG::FindTypedNodeInScene<RootModel>(scene, "ModelRoot");
    box  = Model::CreateModel<BoxModel>("Box");
    cyl  = Model::CreateModel<CylinderModel>("Cyl");
    root->AddChildModel(box);
    root->AddChildModel(cyl);

    // Add all known Tools to the ToolBox.
    const auto tools = FindTools(scene);
    for (auto &tool: tools)
        SetUpTool(tool);
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
}
