#include <ion/math/vectorutils.h>

#include "Items/AppInfo.h"
#include "Items/Border.h"
#include "Items/BuildVolume.h"
#include "Items/Controller.h"
#include "Items/Frame.h"
#include "Items/Inspector.h"
#include "Items/PaneBackground.h"
#include "Items/PrecisionControl.h"
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

    info = ParseObject<AppInfo>(
        "AppInfo { version: \"1.2.3\", session_state: SessionState {} }");
    EXPECT_EQ("1.2.3", info->GetVersion());
    EXPECT_NOT_NULL(info->GetSessionState());

    TEST_THROW(ParseObject<AppInfo>("AppInfo {}"),
               Parser::Exception, "Missing version field data");
    TEST_THROW(ParseObject<AppInfo>("AppInfo { version: \"Something\" }"),
               Parser::Exception, "Missing session_state data");
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
    const std::string contents = R"(
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
    const std::string contents = R"(
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

    TEST_THROW(ParseObject<Frame>("Frame { width: 0 }"),
               Parser::Exception, "Non-positive width or depth");
    TEST_THROW(ParseObject<Frame>("Frame { width: 2, depth: -2 }"),
               Parser::Exception, "Non-positive width or depth");
}

TEST_F(ItemTest, Inspector) {
    // Read the Inspector, a Controller, and a test Node.
    const std::string contents = R"(
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
        ion::math::Normalized(Point3f(inspector->GetTranslation()) -
                              frust.position));

    // Inspector has to be activated for ApplyScaleChange().
    inspector->Activate(node, ControllerPtr());
    const auto size = inspector->GetScaledBounds().GetSize();
    inspector->ApplyScaleChange(1);
    EXPECT_LT(size[0], inspector->GetScaledBounds().GetSize()[0]);
    EXPECT_LT(size[1], inspector->GetScaledBounds().GetSize()[1]);
    EXPECT_LT(size[2], inspector->GetScaledBounds().GetSize()[2]);

    // These have no testable affect.
    inspector->ApplyRotation(BuildRotation(0, 1, 0, 40));
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
