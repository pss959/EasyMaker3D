#include "SG/ColorMap.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/WidgetTestBase.h"
#include "Util/Assert.h"
#include "Widgets/SpinWidget.h"

class CompositeWidgetTest : public WidgetTestBase {
  protected:
    SpinWidgetPtr GetSpinWidget() {
        return GetWidgetOfType<SpinWidget>("nodes/Widgets/SpinWidget.emd",
                                           "SpinWidget");
    }
};

TEST_F(CompositeWidgetTest, GetSubWidget) {
    auto sw = GetSpinWidget();

    // Direct sub-widget
    EXPECT_NOT_NULL(sw->GetSubWidget("Ring"));

    // Nested sub-widget
    EXPECT_NOT_NULL(sw->GetSubWidget("Translator"));

    // Not a sub-widget.
    TEST_THROW(sw->GetSubWidget("NoSuchWidget"),
               AssertException, "No sub-widget");
}

TEST_F(CompositeWidgetTest, Highlight) {
    auto sw = GetSpinWidget();

    auto ring = sw->GetSubWidget("Ring");
    EXPECT_EQ(SG::ColorMap::SGetColor("WidgetActiveColor"),
              ring->GetActiveColor());

    sw->HighlightSubWidget("Ring", Color(1, 0, 0));
    EXPECT_EQ(Color(1, 0, 0), ring->GetActiveColor());

    sw->UnhighlightSubWidget("Ring");
    EXPECT_EQ(SG::ColorMap::SGetColor("WidgetActiveColor"),
              ring->GetActiveColor());
}
