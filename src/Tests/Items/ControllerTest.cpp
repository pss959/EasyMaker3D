#include "Items/Controller.h"
#include "Items/RadialMenu.h"
#include "SG/Search.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Util/Enum.h"
#include "Util/General.h"
#include "Util/Read.h"

/// \ingroup Tests
class ControllerTest : public SceneTestBase {
  protected:
    /// Reads the Controller template if necessary and returns a clone of it
    /// after adding it to the scene (to set up Ion in it) and setting its
    /// Hand.
    ControllerPtr GetController(Hand hand) {
        if (! ct_)
            ct_ = ReadRealNode<Controller>(
                R"(children: [ <"nodes/templates/Controller.emd"> ])",
                "T_Controller");
        auto c = ct_->CloneTyped<Controller>(true, Util::EnumToWord(hand));
        GetScene()->GetRootNode()->AddChild(c);
        c->SetHand(hand);
        return c;
    }

    /// \name Visibility query
    /// Each of these searches in the given Controller instance for a part and
    /// returns whether it is visible (enabled).
    ///@{
    bool IsPointerOn(const Controller &c) { return IsOn_(c, "LaserPointer"); }
    bool IsGripOn(const Controller &c)    { return IsOn_(c, "Grip"); }
    bool IsTouchOn(const Controller &c)   { return IsOn_(c, "Touch"); }
    bool IsPointerHoverOn(const Controller &c) {
        return IsOn_(c, "PointerHoverHighlight");
    }
    bool IsGripHoverOn(const Controller &c) {
        return IsOn_(c, "GripHoverHighlight");
    }
    ///@}

  private:
    ControllerPtr ct_;  ///< Controller template.

    /// Returns true if the named part is enabled.
    bool IsOn_(const Controller &c, const std::string &part) {
        return SG::FindNodeUnderNode(c, part)->IsEnabled();
    }
};

TEST_F(ControllerTest, Default) {
    auto cont = CreateObject<Controller>();
    EXPECT_EQ(Hand::kRight,         cont->GetHand());
    EXPECT_EQ(GripGuideType::kNone, cont->GetGripGuideType());
    EXPECT_EQ(Vector3f(-1, 0, 0),   cont->GetGuideDirection());
    EXPECT_EQ(Vector3f(0, 0, 0),    cont->GetTouchOffset());
    EXPECT_EQ(0,                    cont->GetTouchRadius());

    EXPECT_FALSE(cont->IsInTouchMode());

    ModelMesh mesh;
    ion::gfx::ImagePtr image;
    EXPECT_FALSE(cont->GetCustomModelData(mesh, image));
}

TEST_F(ControllerTest, Hands) {
    auto lc = GetController(Hand::kLeft);
    auto rc = GetController(Hand::kRight);
    EXPECT_EQ(Hand::kLeft,        lc->GetHand());
    EXPECT_EQ(Hand::kRight,       rc->GetHand());
    EXPECT_EQ(Vector3f( 1, 0, 0), lc->GetGuideDirection());
    EXPECT_EQ(Vector3f(-1, 0, 0), rc->GetGuideDirection());
}

TEST_F(ControllerTest, GripGuide) {
    auto lc = GetController(Hand::kLeft);
    auto rc = GetController(Hand::kRight);
    EXPECT_EQ(GripGuideType::kNone,     lc->GetGripGuideType());
    EXPECT_EQ(GripGuideType::kNone,     rc->GetGripGuideType());
    lc->SetGripGuideType(GripGuideType::kBasic);
    rc->SetGripGuideType(GripGuideType::kRotation);
    EXPECT_EQ(GripGuideType::kBasic,    lc->GetGripGuideType());
    EXPECT_EQ(GripGuideType::kRotation, rc->GetGripGuideType());

    // Turn on hovering.
    lc->ShowPointerHover(true, Point3f(0, 0, -100));
    rc->ShowPointerHover(true, Point3f(0, 0, -100));
    lc->ShowGripHover(true, Point3f(0, 0, 0), Color(1, 0, 0));
    rc->ShowGripHover(true, Point3f(0, 0, 0), Color(1, 0, 0));
    EXPECT_TRUE(IsPointerHoverOn(*lc));
    EXPECT_TRUE(IsPointerHoverOn(*rc));
    EXPECT_TRUE(IsGripHoverOn(*lc));
    EXPECT_TRUE(IsGripHoverOn(*rc));
}

TEST_F(ControllerTest, Modes) {
    auto rc = GetController(Hand::kRight);

    // Required for grip hovering.
    rc->SetGripGuideType(GripGuideType::kBasic);

    EXPECT_FALSE(rc->IsInTouchMode());
    EXPECT_TRUE(IsPointerOn(*rc));
    EXPECT_TRUE(IsGripOn(*rc));
    EXPECT_FALSE(IsTouchOn(*rc));
    EXPECT_TRUE(IsPointerHoverOn(*rc));
    EXPECT_FALSE(IsGripHoverOn(*rc));

    // Turn on touch mode.
    rc->SetTouchMode(true);
    EXPECT_TRUE(rc->IsInTouchMode());
    EXPECT_TRUE(IsPointerOn(*rc));
    EXPECT_TRUE(IsGripOn(*rc));
    EXPECT_TRUE(IsTouchOn(*rc));
    EXPECT_TRUE(IsPointerHoverOn(*rc));
    EXPECT_FALSE(IsGripHoverOn(*rc));

    // Turn on hovering.
    rc->ShowPointerHover(true, Point3f(0, 0, -100));
    rc->ShowGripHover(true, Point3f(0, 0, 0), Color(1, 0, 0));
    EXPECT_TRUE(IsPointerHoverOn(*rc));
    EXPECT_TRUE(IsGripHoverOn(*rc));

    // Show all affordances.
    rc->ShowAll(true);
    EXPECT_TRUE(rc->IsInTouchMode());
    EXPECT_TRUE(IsPointerOn(*rc));
    EXPECT_TRUE(IsGripOn(*rc));
    EXPECT_TRUE(IsTouchOn(*rc));
    EXPECT_TRUE(IsPointerHoverOn(*rc));
    EXPECT_TRUE(IsGripHoverOn(*rc));

    // Test all trigger modes.
    rc->SetTriggerMode(Trigger::kPointer, true);
    EXPECT_TRUE(rc->IsInTouchMode());
    EXPECT_TRUE(IsPointerOn(*rc));
    EXPECT_FALSE(IsGripOn(*rc));
    EXPECT_FALSE(IsTouchOn(*rc));
    rc->SetTriggerMode(Trigger::kPointer, false);
    rc->SetTriggerMode(Trigger::kGrip, true);
    EXPECT_TRUE(rc->IsInTouchMode());
    EXPECT_FALSE(IsPointerOn(*rc));
    EXPECT_TRUE(IsGripOn(*rc));
    EXPECT_FALSE(IsTouchOn(*rc));
    rc->SetTriggerMode(Trigger::kGrip, false);
    rc->SetTriggerMode(Trigger::kTouch, true);
    EXPECT_TRUE(rc->IsInTouchMode());
    EXPECT_FALSE(IsPointerOn(*rc));
    EXPECT_FALSE(IsGripOn(*rc));
    EXPECT_TRUE(IsTouchOn(*rc));
    rc->SetTriggerMode(Trigger::kTouch, false);
    // Back to normal.
    EXPECT_TRUE(rc->IsInTouchMode());
    EXPECT_TRUE(IsPointerOn(*rc));
    EXPECT_TRUE(IsGripOn(*rc));
    EXPECT_TRUE(IsTouchOn(*rc));
    EXPECT_TRUE(IsPointerHoverOn(*rc));
    EXPECT_TRUE(IsGripHoverOn(*rc));

    // Turn off hovering.
    rc->ShowPointerHover(false, Point3f(0, 0, 0));
    rc->ShowGripHover(false, Point3f(0, 0, 0), Color(0, 0, 0));
    EXPECT_FALSE(IsPointerHoverOn(*rc));
    EXPECT_FALSE(IsGripHoverOn(*rc));
}

TEST_F(ControllerTest, ShowTouchAndVibrate) {
    float seconds = 0;
    auto vib_func = [&](float secs){ seconds = secs; };

    auto rc = GetController(Hand::kRight);
    rc->SetVibrateFunc(vib_func);
    EXPECT_EQ(0, seconds);
    rc->ShowTouch(true);
    EXPECT_LT(0, seconds);
    const float prev_seconds = seconds;
    rc->ShowTouch(false);
    EXPECT_EQ(prev_seconds, seconds);
}

TEST_F(ControllerTest, CustomModel) {
    const auto test_hand = [&](Hand hand, const std::string &name){
        auto c = GetController(hand);

        // Read the Controller model data.
        const std::string model = "models/controllers/Vive_" + name;
        const std::string mesh_file = model + ".tri";
        const std::string tex_file  = model + ".jpg";
        std::string mesh_data;
        Controller::CustomModel cm;
        EXPECT_TRUE(Util::ReadFile(mesh_file, mesh_data));
        EXPECT_TRUE(cm.mesh.FromBinaryString(mesh_data));
        cm.texture_image = Util::ReadImage(tex_file, false);
        EXPECT_NOT_NULL(cm.texture_image.Get());

        c->UseCustomModel(cm);

        ModelMesh mesh;
        ion::gfx::ImagePtr image;
        EXPECT_TRUE(c->GetCustomModelData(mesh, image));
    };

    test_hand(Hand::kLeft,  "Left");
    test_hand(Hand::kRight, "Right");
}

TEST_F(ControllerTest, Attach) {
    auto box = ParseObject<SG::Node>("Node { shapes: [ Box {} ] }");
    const Vector3f  box_trans(10, 20, 30);
    const Rotationf box_rot = BuildRotation(0, 1, 0, 45);
    box->SetTranslation(box_trans);
    box->SetRotation(box_rot);

    auto lc = GetController(Hand::kLeft);
    auto rc = GetController(Hand::kRight);

    lc->SetTranslation(Vector3f(3, 4, 5));
    rc->SetTranslation(Vector3f(3, 4, 5));
    lc->SetRotation(BuildRotation(1, 0, 0, 10));
    rc->SetRotation(BuildRotation(1, 0, 0, 10));

    lc->AttachObject(box, Vector3f(1, 2, 3));  // Offset.
    EXPECT_TRUE(Util::Contains(lc->GetChildren(), box));
    EXPECT_EQ(Vector3f(1, 2, 3),           box->GetTranslation());
    EXPECT_EQ(BuildRotation(1, 0, 0, -90), box->GetRotation());
    lc->DetachObject(box);
    EXPECT_FALSE(Util::Contains(lc->GetChildren(), box));

    // Offset is negated in X for right controller.
    rc->AttachObject(box, Vector3f(1, 2, 3));  // Offset.
    EXPECT_TRUE(Util::Contains(rc->GetChildren(), box));
    EXPECT_EQ(Vector3f(-1, 2, 3),          box->GetTranslation());
    EXPECT_EQ(BuildRotation(1, 0, 0, -90), box->GetRotation());
    rc->DetachObject(box);
    EXPECT_FALSE(Util::Contains(rc->GetChildren(), box));
}

TEST_F(ControllerTest, AttachRadialMenu) {
    // Need to add the controller and the RadialMenu.
    const std::string contents = R"(
  children: [
    Node {
      TEMPLATES: [
         <"nodes/templates/RadialMenu.emd">,
         <"nodes/templates/Controller.emd">,
      ],
      children: [
         CLONE "T_RadialMenu" "TestMenu"  {},
         CLONE "T_Controller" "TestLeft"  {},
         CLONE "T_Controller" "TestRight" {},
      ],
    }
  ]
)";
    auto scene = ReadRealScene(contents);

    auto menu = SG::FindTypedNodeInScene<RadialMenu>(*scene, "TestMenu");
    auto lc   = SG::FindTypedNodeInScene<Controller>(*scene, "TestLeft");
    auto rc   = SG::FindTypedNodeInScene<Controller>(*scene, "TestRight");
    lc->SetHand(Hand::kLeft);
    rc->SetHand(Hand::kRight);

    lc->AttachRadialMenu(menu);
    EXPECT_NOT_NULL(SG::FindNodeUnderNode(*lc, "TestMenu"));

    rc->AttachRadialMenu(menu);
    EXPECT_NOT_NULL(SG::FindNodeUnderNode(*rc, "TestMenu"));
}
