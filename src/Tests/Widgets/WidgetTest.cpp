#include "Place/DragInfo.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Util/Assert.h"
#include "Widgets/Widget.h"

class WidgetTest : public SceneTestBase {
  protected:
    class TestWidget : public Widget {
      public:
        void AllowActiveHovering() { can_active_hover_ = true; }
      protected:
        TestWidget() {}
        // Allows active hovering to work.
        virtual bool SupportsActiveHovering() override {
            return can_active_hover_ || Widget::SupportsActiveHovering();
        }
      private:
        bool can_active_hover_ = false;
        friend class Parser::Registry;
    };
    typedef std::shared_ptr<TestWidget> TestWidgetPtr;

    WidgetTest();
    TestWidgetPtr CreateTestWidget(bool set_up_ion);

  private:
    SG::ScenePtr scene_;  // Saves scene from CreateTestWidget().
};

WidgetTest::WidgetTest() {
    Parser::Registry::AddType<TestWidget>("TestWidget");
}

WidgetTest::TestWidgetPtr WidgetTest::CreateTestWidget(bool set_up_ion) {
    // Create a TestWidget with hover scaling enabled.
    const std::string input = "TestWidget \"TW\" { hover_scale: 2 2 2 }";
    auto tw = ReadTypedItem<TestWidget>(input);
    EXPECT_NOT_NULL(tw);
    EXPECT_EQ("TW", tw->GetName());

    if (set_up_ion) {
       const std::string input = ReadDataFile("RealScene");
        scene_ = ReadScene(input, true);
        // This will set up Ion in the TestWidget.
        scene_->GetRootNode()->AddChild(tw);
    }
    return tw;
}

// ----------------------------------------------------------------------------
// Tests.
// ----------------------------------------------------------------------------

TEST_F(WidgetTest, InteractionEnabled) {
    auto tw = CreateTestWidget(true);  // Need true to test color setting.
    EXPECT_TRUE(tw->IsInteractionEnabled());
    tw->SetInteractionEnabled(false);
    EXPECT_FALSE(tw->IsInteractionEnabled());
    tw->SetInteractionEnabled(true);
    EXPECT_TRUE(tw->IsInteractionEnabled());
}

TEST_F(WidgetTest, Active) {
    auto tw = CreateTestWidget(false);

    // Set a function to invoke when activating or deactivating.
    std::string activation_string = "X";
    const auto func = [&](Widget &w, bool is_act){
        activation_string += is_act ? "A" : "D";
    };
    tw->GetActivation().AddObserver("key", func);

    EXPECT_FALSE(tw->IsActive());
    tw->SetActive(true, true);  // Notify.
    EXPECT_TRUE(tw->IsActive());
    EXPECT_EQ("XA", activation_string);
    tw->SetActive(false, true);  // Notify.
    EXPECT_FALSE(tw->IsActive());
    EXPECT_EQ("XAD", activation_string);

    tw->SetActive(false, false);  // Do not notify.
    EXPECT_FALSE(tw->IsActive());
    EXPECT_EQ("XAD", activation_string);
    tw->SetActive(true, false);  // Do not notify.
    EXPECT_TRUE(tw->IsActive());
    EXPECT_EQ("XAD", activation_string);
    tw->SetActive(false, false);  // Do not notify.

    // No change if interaction is disabled.
    tw->SetInteractionEnabled(false);
    tw->SetActive(true, true);  // Notify.
    EXPECT_FALSE(tw->IsActive());
    EXPECT_EQ("XAD", activation_string);
}

TEST_F(WidgetTest, Hover) {
    // Changing colors in a Widget requires a scene with RenderPasses set up.
    auto tw = CreateTestWidget(true);

    EXPECT_FALSE(tw->IsHovering());
    tw->StartHovering();
    EXPECT_TRUE(tw->IsHovering());
    tw->StopHovering();
    EXPECT_FALSE(tw->IsHovering());

    // Hover multiple times (as for different devices).
    tw->StartHovering();
    EXPECT_TRUE(tw->IsHovering());
    tw->StartHovering();
    EXPECT_TRUE(tw->IsHovering());
    tw->StartHovering();
    EXPECT_TRUE(tw->IsHovering());
    tw->StopHovering();
    EXPECT_TRUE(tw->IsHovering());
    tw->StopHovering();
    EXPECT_TRUE(tw->IsHovering());
    tw->StopHovering();
    EXPECT_FALSE(tw->IsHovering());

    // Setting to active should stop hovering.
    EXPECT_FALSE(tw->IsActive());
    tw->StartHovering();
    EXPECT_TRUE(tw->IsHovering());
    tw->SetActive(true);
    EXPECT_FALSE(tw->IsHovering());
    tw->SetActive(false);

    // Test that the hover scale is applied while hovering.
    EXPECT_EQ(Vector3f(1, 1, 1), tw->GetScale());
    tw->StartHovering();
    EXPECT_TRUE(tw->IsHovering());
    EXPECT_EQ(Vector3f(2, 2, 2), tw->GetScale());
    tw->StopHovering();
    EXPECT_FALSE(tw->IsHovering());
    EXPECT_EQ(Vector3f(1, 1, 1), tw->GetScale());

    // Hover should have no visible effect while active unless
    // SupportsActiveHovering() returns true. Use the scaling to check.
    tw->SetActive(true);
    tw->StartHovering();
    EXPECT_TRUE(tw->IsHovering());
    EXPECT_EQ(Vector3f(1, 1, 1), tw->GetScale());
    tw->StopHovering();
    EXPECT_FALSE(tw->IsHovering());
    EXPECT_EQ(Vector3f(1, 1, 1), tw->GetScale());

    // Try with SupportsActiveHovering() returning true: scale should change.
    tw->AllowActiveHovering();
    tw->StartHovering();
    EXPECT_TRUE(tw->IsHovering());
    EXPECT_EQ(Vector3f(2, 2, 2), tw->GetScale());
    tw->StopHovering();
    EXPECT_FALSE(tw->IsHovering());
    EXPECT_EQ(Vector3f(1, 1, 1), tw->GetScale());

    // This should have no effect.
    tw->UpdateHoverPoint(Point3f(1, 2, 3));
}

TEST_F(WidgetTest, Colors) {
    auto tw = CreateTestWidget(true);

    // Default colors.
    EXPECT_EQ(SG::ColorMap::SGetColor("WidgetActiveColor"),
              tw->GetActiveColor());
    EXPECT_EQ(SG::ColorMap::SGetColor("WidgetInactiveColor"),
              tw->GetInactiveColor());

    // Colors with a different prefix.
    tw->SetColorNamePrefix("Target");
    EXPECT_EQ(SG::ColorMap::SGetColor("TargetActiveColor"),
              tw->GetActiveColor());
    EXPECT_EQ(SG::ColorMap::SGetColor("TargetInactiveColor"),
              tw->GetInactiveColor());

    // Set specific colors.
    tw->SetActiveColor(Color(1, 0, 1));
    EXPECT_EQ(Color(1, 0, 1), tw->GetActiveColor());
    EXPECT_EQ(SG::ColorMap::SGetColor("TargetInactiveColor"),
              tw->GetInactiveColor());
    tw->SetInactiveColor(Color(0, 1, 1));
    EXPECT_EQ(Color(1, 0, 1), tw->GetActiveColor());
    EXPECT_EQ(Color(0, 1, 1), tw->GetInactiveColor());
}

TEST_F(WidgetTest, Tooltip) {
    auto tw = CreateTestWidget(true);
    std::string s;
    const auto func = [&](Widget &w, const std::string &str, bool show){
        EXPECT_EQ(&w, tw.get());
        if (show)
            s += str;
        else
            s.clear();
    };
    tw->SetTooltipFunc(func);
    tw->SetTooltipText("Testing");

    // Activate the tooltip by hovering. Only the first hover should do so.
    EXPECT_EQ("", s);
    tw->StartHovering();
    EXPECT_EQ("Testing", s);
    tw->StartHovering();
    EXPECT_EQ("Testing", s);
    tw->StopHovering();
    EXPECT_EQ("Testing", s);
    tw->StopHovering();
    EXPECT_EQ("", s);
}

TEST_F(WidgetTest, PlaceTarget) {
    auto tw = CreateTestWidget(false);
    EXPECT_FALSE(tw->CanReceiveTarget());

    // Base Widget class does not support target placement.
    Point3f        pos0, pos1;
    Vector3f       dir;
    Dimensionality dims;
    TEST_THROW(tw->PlacePointTarget(DragInfo(), pos0, dir, dims),
               AssertException, "should not be called");
    TEST_THROW(tw->PlaceEdgeTarget(DragInfo(), 10, pos0, pos1),
               AssertException, "should not be called");
}
