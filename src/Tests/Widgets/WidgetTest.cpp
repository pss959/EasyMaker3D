#include "Place/DragInfo.h"
#include "Place/TouchInfo.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing2.h"
#include "Util/Assert.h"
#include "Widgets/Widget.h"

/// Class that defines and sets up a TestWidget type.
/// \ingroup Tests
class WidgetTest : public SceneTestBase {
  protected:
    /// Derived Widget class that allows changes to active hovering.
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
    using TestWidgetPtr = std::shared_ptr<TestWidget>;

    WidgetTest() {
        Parser::Registry::AddType<TestWidget>("TestWidget");
    }

    /// Creates a TestWidget with hover scaling enabled.
    TestWidgetPtr CreateTestWidget() {
        return ReadRealNode<TestWidget>(
            R"(children: [TestWidget "TW" { hover_scale: 2 2 2 } ])", "TW");
    }
};

// ----------------------------------------------------------------------------
// Tests.
// ----------------------------------------------------------------------------

TEST_F(WidgetTest, InteractionEnabled) {
    auto tw = CreateTestWidget();
    EXPECT_TRUE(tw->IsInteractionEnabled());
    tw->SetInteractionEnabled(false);
    EXPECT_FALSE(tw->IsInteractionEnabled());
    tw->SetInteractionEnabled(true);
    EXPECT_TRUE(tw->IsInteractionEnabled());
}

TEST_F(WidgetTest, Active) {
    auto tw = CreateTestWidget();

    // Set a function to invoke when activating or deactivating.
    Str activation_string = "X";
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
    auto tw = CreateTestWidget();

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
    auto tw = CreateTestWidget();

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
    auto tw = CreateTestWidget();
    Str s;
    const auto func = [&](Widget &w, const Str &str, bool show){
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

TEST_F(WidgetTest, IsTouched) {
    auto tw = CreateTestWidget();

    TouchInfo info;
    float     dist = 1000;

    // Cannot touch a non-interactive Widget.
    tw->SetInteractionEnabled(false);
    EXPECT_FALSE(tw->IsTouched(info, dist));

    tw->SetInteractionEnabled(true);

    // Miss the Widget.
    info.root_node = tw;
    info.position.Set(100, 0, 0);
    info.radius = 1;
    EXPECT_FALSE(tw->IsTouched(info, dist));

    // Intersect the Widget.
    info.position.Set(0, 0, 0);
    EXPECT_TRUE(tw->IsTouched(info, dist));
}
