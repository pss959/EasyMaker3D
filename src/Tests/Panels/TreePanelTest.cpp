#include "Agents/ActionAgent.h"
#include "Managers/SelectionManager.h"
#include "Models/BoxModel.h"
#include "Models/HullModel.h"
#include "Models/RootModel.h"
#include "Panels/TreePanel.h"
#include "Panes/TextPane.h"
#include "SG/ColorMap.h"
#include "Tests/Panels/PanelTestBase.h"
#include "Tests/Testing.h"
#include "Util/String.h"

// ----------------------------------------------------------------------------
// TreePanelTest class.
// ----------------------------------------------------------------------------

/// \ingroup Tests
class TreePanelTest : public PanelTestBase {
  protected:
    /// Model hierarchy returned by BuildModelHierarchy().
    struct Hierarchy {
        RootModelPtr root;
        HullModelPtr hull0, hull1;
        BoxModelPtr  box0, box1, box2, box3;
    };

    /// Derived ActionAgent that allows for testing of Action processing.
    class TestActionAgent : public ActionAgent {
      public:
        virtual void Reset() override {};
        virtual void UpdateFromSessionState(
            const SessionState &state) override {};
        virtual bool CanApplyAction(Action action) const override {
            return false; // XXXX
        }
        virtual void ApplyAction(Action action) override {
            // XXXX
        }
    };
    DECL_SHARED_PTR(TestActionAgent);

    /// Derived SelectionAgent that allows for testing of Selection processing.
    /// XXXX Use SelectionManager as is?
    class TestSelectionAgent : public SelectionManager {};
    DECL_SHARED_PTR(TestSelectionAgent);

    TreePanelPtr          panel;
    TestActionAgentPtr    action_agent;
    TestSelectionAgentPtr selection_agent;

    TreePanelTest();

    /// Creates and returns a Hierarchy of Models to use for testing.
    Hierarchy BuildModelHierarchy();

    /// Tests that the indexed Model row in the TreePanel has a button for the
    /// named Model and uses the given color.
    void TestModelRow(int row_index, const Str &name, const Str &color_name);
};

TreePanelTest::TreePanelTest() : action_agent(new TestActionAgent),
                                 selection_agent(new TestSelectionAgent) {
    GetContext().action_agent    = action_agent;
    GetContext().selection_agent = selection_agent;
    panel = InitPanel<TreePanel>("TreePanel");
}

TreePanelTest::Hierarchy TreePanelTest::BuildModelHierarchy() {
    Hierarchy hier;
    hier.root  = Model::CreateModel<RootModel>("Root");
    hier.hull0 = Model::CreateModel<HullModel>("Hull0");
    hier.hull1 = Model::CreateModel<HullModel>("Hull1");
    hier.box0  = Model::CreateModel<BoxModel>("Box0");
    hier.box1  = Model::CreateModel<BoxModel>("Box1");
    hier.box2  = Model::CreateModel<BoxModel>("Box2");
    hier.box3  = Model::CreateModel<BoxModel>("Box3");

    hier.hull0->SetOperandModels(std::vector<ModelPtr>{ hier.box0, hier.box1 });
    hier.hull1->SetOperandModels(std::vector<ModelPtr>{ hier.box2, hier.box3 });
    hier.root->AddChildModel(hier.hull0);
    hier.root->AddChildModel(hier.hull1);

    return hier;
}

void TreePanelTest::TestModelRow(int row_index, const Str &name,
                                 const Str &color_name){
    SCOPED_TRACE("Row " + Util::ToString(row_index));
    auto row = FindPane("ModelRow_" + Util::ToString(row_index));
    ASSERT_NOT_NULL(row);
    auto but = row->FindSubPane("ModelButton");
    EXPECT_NOT_NULL(but);
    auto text = but->FindTypedSubPane<TextPane>("Text");
    ASSERT_NOT_NULL(text);
    EXPECT_EQ(name, text->GetText());
    EXPECT_EQ(SG::ColorMap::SGetColor("TreePanel" + color_name + "Color"),
              text->GetColor());
};

// ----------------------------------------------------------------------------
// TreePanelTest tests.
// ----------------------------------------------------------------------------

TEST_F(TreePanelTest, Defaults) {
    EXPECT_NULL(panel->GetFocusedPane());
    EXPECT_EQ(".", FindTypedPane<TextPane>("SessionString")->GetText());
}

TEST_F(TreePanelTest, Show) {
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    panel->SetStatus(Panel::Status::kVisible);
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());
    EXPECT_EQ(FindPane("HideButton"), panel->GetFocusedPane());
}

TEST_F(TreePanelTest, Models) {
    const auto hier = BuildModelHierarchy();

    panel->SetSessionString("Session");
    EXPECT_EQ("Session", FindTypedPane<TextPane>("SessionString")->GetText());

    panel->SetRootModel(hier.root);
    panel->SetStatus(Panel::Status::kVisible);

    // Call this so the TreePanel builds the Model rows.
    panel->UpdateForRenderPass("");

    // These are in hierarchy order.
    TestModelRow(0, "Hull0", "Default");
    TestModelRow(1, "Box0",  "HiddenByModel");
    TestModelRow(2, "Box1",  "HiddenByModel");
    TestModelRow(3, "Hull1", "Default");
    TestModelRow(4, "Box2",  "HiddenByModel");
    TestModelRow(5, "Box3",  "HiddenByModel");

    // Remove a model and update. The rows should be modified.
    hier.hull1->RemoveChildModel(0);
    panel->ModelsChanged();
    panel->UpdateForRenderPass("");
    TestModelRow(0, "Hull0", "Default");
    TestModelRow(1, "Box0",  "HiddenByModel");
    TestModelRow(2, "Box1",  "HiddenByModel");
    TestModelRow(3, "Hull1", "Default");
    TestModelRow(4, "Box3",  "HiddenByModel");
}

// XXXX Test selection.
