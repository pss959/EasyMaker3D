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
    EXPECT_TRUE(panel->IsMovable());
    EXPECT_TRUE(panel->IsResizable());
    EXPECT_FALSE(panel->CanGripHover());
    EXPECT_NULL(panel->GetGripWidget(Point2f(0, 0)));
}

TEST_F(InfoPanelTest, Show) {
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());

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

    panel->SetStatus(Panel::Status::kVisible);
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());
    EXPECT_EQ(FindPane("Done"), panel->GetFocusedPane());

    ClickButtonPane("Done");
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    EXPECT_EQ("Done", GetCloseResult());
}
