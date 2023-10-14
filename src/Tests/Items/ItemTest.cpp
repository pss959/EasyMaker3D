#include "Items/AppInfo.h"
#include "Items/Border.h"
#include "Items/BuildVolume.h"
#include "Items/Controller.h"
#include "Items/Frame.h"
#include "Items/Inspector.h"
#include "Items/PaneBackground.h"
#include "Items/PrecisionControl.h"
#include "Items/SessionState.h"
#include "Items/Shelf.h"
#include "Items/UnitConversion.h"
#include "Parser/Exception.h"
#include "SG/Search.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Util/Tuning.h"
#include "Widgets/IconWidget.h"

// Most Items are simple enough that they can all be tested in one file.

/// \ingroup Tests
class ItemTest : public SceneTestBase {};

TEST_F(ItemTest, AppInfo) {
    auto info = CreateObject<AppInfo>();
    EXPECT_TRUE(info->GetVersion().empty());
    EXPECT_NULL(info->GetSessionState());

    info = AppInfo::CreateDefault();
    EXPECT_EQ(TK::kVersionString, info->GetVersion());
    EXPECT_NOT_NULL(info->GetSessionState());

    info = ParseTypedObject<AppInfo>(
        "AppInfo { version: \"1.2.3\", session_state: SessionState {} }");
    EXPECT_EQ("1.2.3", info->GetVersion());
    EXPECT_NOT_NULL(info->GetSessionState());

    SetParseTypeName("AppInfo");
    TestInvalid("", "Missing version field data");
    TestInvalid(R"(version: "Something")", "Missing session_state data");
}

TEST_F(ItemTest, Border) {
    auto border = CreateObject<Border>();
    EXPECT_EQ(Color::Black(), border->GetColor());
    EXPECT_EQ(1,              border->GetWidth());

    border->SetColor(Color(1, 1, 0));
    border->SetWidth(2.5f);
    EXPECT_EQ(Color(1, 1, 0), border->GetColor());
    EXPECT_EQ(2.5f,           border->GetWidth());

    // Size is 0 until SetUpIon() is called.
    auto size = border->GetScaledBounds().GetSize();
    EXPECT_EQ(0, size[0]);
    EXPECT_EQ(0, size[1]);
    border->SetSize(Vector2f(20, 10));
    EXPECT_EQ(0, size[0]);
    EXPECT_EQ(0, size[1]);

    // This calls SetUpIon(). Note that the Border needs to have a
    // MutableTriMeshShape for this to work.
    const Str contents = R"(
  children: [
    Border "TestBorder" { shapes: [ MutableTriMeshShape {} ] }
  ],
)";
    border = ReadRealNode<Border>(contents, "TestBorder");
    border->SetSize(Vector2f(20, 10));
    size = border->GetScaledBounds().GetSize();
    // There should be a real mesh installed with size 1x1. (The actual size is
    // created with Pane scaling; the passed-in size is to handle width
    // correctly.)
    EXPECT_EQ(1, size[0]);
    EXPECT_EQ(1, size[1]);

    // Width = 0 causes the border to be disabled.
    EXPECT_TRUE(border->IsEnabled());
    border->SetWidth(0);
    EXPECT_FALSE(border->IsEnabled());
}

TEST_F(ItemTest, BuildVolume) {
    auto bv = CreateObject<BuildVolume>();
    EXPECT_EQ(Vector3f(1, 1, 1), bv->GetSize());
    EXPECT_TRUE(bv->IsActive());

    bv->SetSize(Vector3f(20, 30, 40));
    EXPECT_EQ(Vector3f(20, 30, 40), bv->GetSize());
    EXPECT_TRUE(bv->IsActive());

    bv->Activate(false);
    EXPECT_EQ(Vector3f(20, 30, 40), bv->GetSize());
    EXPECT_FALSE(bv->IsActive());

    bv->Activate(true);
    EXPECT_TRUE(bv->IsActive());
    EXPECT_EQ(Vector3f(20, 30, 40), bv->GetScale());
    EXPECT_EQ(Vector3f(0,  15,  0), bv->GetTranslation());
}

TEST_F(ItemTest, Frame) {
    auto frame = CreateObject<Frame>();
    EXPECT_NULL(frame->GetFramed());
    EXPECT_EQ(Vector3f(0, 0, 0), frame->GetBounds().GetSize());

    // Create a frame around a Box.
    const Str contents = R"(
  children: [
    Node {
      TEMPLATES: [ <"nodes/templates/Frame.emd"> ],
      children: [
        CLONE "T_Frame" "TestFrame" {
          width:  2,
          depth:  3,
          framed: Node "Box" {
            shapes: [ Box { size: 10 20 30 } ]
          },
        }
      ],
    }
  ]
    )";

    frame = ReadRealNode<Frame>(contents, "TestFrame");

    EXPECT_NOT_NULL(frame->GetFramed());
    EXPECT_EQ("Box", frame->GetFramed()->GetName());
    EXPECT_VECS_CLOSE(Vector3f(14, 24, 3), frame->GetBounds().GetSize());

    SetParseTypeName("Frame");
    TestInvalid("width: 0",            "Non-positive width or depth");
    TestInvalid("width: 2, depth: -2", "Non-positive width or depth");
}

TEST_F(ItemTest, Inspector) {
    // Read the Inspector, a Controller, and a test Node.
    const Str contents = R"(
  children: [
    Node {
      TEMPLATES: [<"nodes/templates/Controller.emd">],
      children: [
         CLONE "T_Controller" "TCont" {},
         <"nodes/Inspector.emd">,
        Node "TestNode" { shapes: [ Box { size: 3 4 5 } ] },
      ],
    }
  ]
)";

    auto scene = ReadRealScene(contents);
    auto inspector  = SG::FindTypedNodeInScene<Inspector>(*scene,  "Inspector");
    auto controller = SG::FindTypedNodeInScene<Controller>(*scene, "TCont");
    auto node       = SG::FindNodeInScene(*scene,                  "TestNode");

    size_t deact_count = 0;
    auto deact = [&](){ ++deact_count; };
    inspector->SetDeactivationFunc(deact);

    EXPECT_FALSE(inspector->IsEnabled());
    inspector->Activate(node, ControllerPtr());
    EXPECT_TRUE(inspector->IsEnabled());
    EXPECT_EQ(0U, deact_count);
    inspector->Deactivate();
    EXPECT_FALSE(inspector->IsEnabled());
    EXPECT_EQ(1U, deact_count);

    Frustum frust;
    inspector->SetPositionForView(frust);
    EXPECT_VECS_CLOSE(
        frust.GetViewDirection(),
        Normalized(Point3f(inspector->GetTranslation()) - frust.position));

    EXPECT_EQ(1,                     inspector->GetCurrentScale());
    EXPECT_EQ(Rotationf::Identity(), inspector->GetCurrentRotation());

    // Inspector has to be activated for ApplyScaleChange().
    inspector->Activate(node, ControllerPtr());
    const auto size = inspector->GetScaledBounds().GetSize();
    inspector->ApplyScaleChange(1);
    EXPECT_CLOSE(1.06f,              inspector->GetCurrentScale());
    EXPECT_EQ(Rotationf::Identity(), inspector->GetCurrentRotation());
    EXPECT_LT(size[0], inspector->GetScaledBounds().GetSize()[0]);
    EXPECT_LT(size[1], inspector->GetScaledBounds().GetSize()[1]);
    EXPECT_LT(size[2], inspector->GetScaledBounds().GetSize()[2]);

    const auto rot = BuildRotation(0, 1, 0, 40);
    inspector->ApplyRotation(rot);
    EXPECT_CLOSE(1.06f, inspector->GetCurrentScale());
    EXPECT_EQ(rot,      inspector->GetCurrentRotation());

    // This has no testable effect.
    inspector->ShowEdges(true);

    inspector->Deactivate();
    EXPECT_EQ(2U, deact_count);

    // Attach to a controller.
    inspector->Activate(node, controller);
    inspector->Deactivate();
    EXPECT_EQ(3U, deact_count);
}

TEST_F(ItemTest, PaneBackground) {
    auto pbg = CreateObject<PaneBackground>();
    EXPECT_EQ(Color::Black(), pbg->GetColor());

    pbg = ReadRealNode<PaneBackground>(
        R"(children: [ PaneBackground "BG" { color: "#ffff00ff" } ] )", "BG");
    EXPECT_EQ(Color(1, 1, 0), pbg->GetColor());
}

TEST_F(ItemTest, PrecisionControl) {
    auto pc = ReadRealNode<PrecisionControl>(
        R"(children: [ <"nodes/PrecisionControl.emd">])", "PrecisionControl");
    EXPECT_EQ(2U, pc->GetIcons().size());
    EXPECT_EQ("IncreasePrecision", pc->GetIcons()[0]->GetName());
    EXPECT_EQ("DecreasePrecision", pc->GetIcons()[1]->GetName());

    // This has no testable affect.
    pc->Update(.1f, 5);
}

TEST_F(ItemTest, SessionState) {
    auto ss = CreateObject<SessionState>();

    EXPECT_FALSE(ss->IsPointTargetVisible());
    EXPECT_FALSE(ss->IsEdgeTargetVisible());
    EXPECT_FALSE(ss->AreEdgesShown());
    EXPECT_FALSE(ss->IsBuildVolumeVisible());
    EXPECT_FALSE(ss->IsAxisAligned());

    ss->SetPointTargetVisible(true);
    ss->SetEdgeTargetVisible(true);
    ss->SetEdgesShown(true);
    ss->SetBuildVolumeVisible(true);
    ss->SetAxisAligned(true);
    EXPECT_TRUE(ss->IsPointTargetVisible());
    EXPECT_TRUE(ss->IsEdgeTargetVisible());
    EXPECT_TRUE(ss->AreEdgesShown());
    EXPECT_TRUE(ss->IsBuildVolumeVisible());
    EXPECT_TRUE(ss->IsAxisAligned());

    ss->SetEdgesShown(false);
    auto copy = CreateObject<SessionState>();
    copy->CopyFrom(*ss);
    EXPECT_TRUE(copy->IsPointTargetVisible());
    EXPECT_TRUE(copy->IsEdgeTargetVisible());
    EXPECT_FALSE(copy->AreEdgesShown());
    EXPECT_TRUE(copy->IsBuildVolumeVisible());
    EXPECT_TRUE(copy->IsAxisAligned());
    EXPECT_TRUE(copy->IsSameAs(*ss));
    copy->SetAxisAligned(false);
    EXPECT_FALSE(copy->IsSameAs(*ss));
}

TEST_F(ItemTest, UnitConversion) {
    auto uc = CreateObject<UnitConversion>();
    EXPECT_EQ(UnitConversion::Units::kCentimeters, uc->GetFromUnits());
    EXPECT_EQ(UnitConversion::Units::kCentimeters, uc->GetToUnits());
    EXPECT_EQ(1, uc->GetFactor());

    uc->SetFromUnits(UnitConversion::Units::kMeters);
    EXPECT_EQ(UnitConversion::Units::kMeters,      uc->GetFromUnits());
    EXPECT_EQ(UnitConversion::Units::kCentimeters, uc->GetToUnits());
    EXPECT_EQ(100, uc->GetFactor());

    uc->SetToUnits(UnitConversion::Units::kMillimeters);
    EXPECT_EQ(UnitConversion::Units::kMeters,      uc->GetFromUnits());
    EXPECT_EQ(UnitConversion::Units::kMillimeters, uc->GetToUnits());
    EXPECT_EQ(1000, uc->GetFactor());

    auto copy = CreateObject<UnitConversion>();
    copy->CopyFrom(*uc);
    EXPECT_EQ(UnitConversion::Units::kMeters,      copy->GetFromUnits());
    EXPECT_EQ(UnitConversion::Units::kMillimeters, copy->GetToUnits());
    EXPECT_EQ(1000, copy->GetFactor());

    EXPECT_EQ(1, UnitConversion::GetConversionFactor(
                  UnitConversion::Units::kCentimeters));
    EXPECT_EQ(10, UnitConversion::GetConversionFactor(
                  UnitConversion::Units::kMillimeters));
    EXPECT_EQ(.01f, UnitConversion::GetConversionFactor(
                  UnitConversion::Units::kMeters));
    EXPECT_CLOSE(.393701f, UnitConversion::GetConversionFactor(
                  UnitConversion::Units::kInches));
    EXPECT_CLOSE(.0328084f, UnitConversion::GetConversionFactor(
                     UnitConversion::Units::kFeet));
}

TEST_F(ItemTest, Shelf) {
    auto shelf = CreateObject<Shelf>();
    EXPECT_TRUE(shelf->GetIcons().empty());

    const Str contents = R"(
  children: [
    <"nodes/Shelf.emd">,
    CLONE "Shelf" "TestShelf" {
      icons: [
        IconWidget "Icon0" { shapes: [ Box { size: 1 1 1 } ] },
        IconWidget "Icon1" { shapes: [ Box { size: 2 2 2 } ] },
        IconWidget "Icon2" { shapes: [ Box { size: 4 4 4 } ] },
      ]
    }
  ],
)";
    shelf = ReadRealNode<Shelf>(contents, "TestShelf");
    EXPECT_EQ(3U, shelf->GetIcons().size());
    shelf->LayOutIcons(Point3f(0, 0, 100));
}
