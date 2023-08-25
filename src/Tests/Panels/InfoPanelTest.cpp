#include "Models/BoxModel.h"
#include "Models/ImportedModel.h"
#include "Models/RootModel.h"
#include "Panels/InfoPanel.h"
#include "Place/EdgeTarget.h"
#include "Place/PointTarget.h"
#include "Selection/SelPath.h"
#include "Selection/Selection.h"
#include "Tests/Panels/PanelTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class InfoPanelTest : public PanelTestBase {
  protected:
    InfoPanelPtr panel;
    InfoPanelTest() { panel = InitPanel<InfoPanel>("InfoPanel"); }
};

TEST_F(InfoPanelTest, Defaults) {
    EXPECT_NULL(panel->GetFocusedPane());
}

TEST_F(InfoPanelTest, Show) {
    EXPECT_FALSE(panel->IsShown());

    // Create two Models, one with an invalid mesh.
    auto root = Model::CreateModel<RootModel>("Root");
    auto box  = Model::CreateModel<BoxModel>("Box");
    auto imp  = Model::CreateModel<ImportedModel>("Imp");
    imp->SetPath(GetDataPath("unclosed.stl").ToString());
    root->AddChildModel(box);
    root->AddChildModel(imp);

    // Create a selection containing both Models.
    Selection sel(SelPath(root, box));
    sel.Add(SelPath(root, imp));

    // Create target instances.
    auto et = CreateObject<EdgeTarget>("ET");
    auto pt = CreateObject<PointTarget>("PT");

    InfoPanel::Info info;
    info.selection    = sel;
    info.point_target = pt.get();
    info.edge_target  = et.get();
    panel->SetInfo(info);

    panel->SetIsShown(true);
    EXPECT_TRUE(panel->IsShown());
    EXPECT_EQ(FindPane("Done"), panel->GetFocusedPane());

    ClickButtonPane("Done");
    EXPECT_FALSE(panel->IsShown());
    EXPECT_EQ("Done", GetCloseResult());
}
