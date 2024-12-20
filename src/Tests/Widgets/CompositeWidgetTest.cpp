//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "SG/ColorMap.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing2.h"
#include "Util/Assert.h"
#include "Widgets/SpinWidget.h"

/// \ingroup Tests
class CompositeWidgetTest : public SceneTestBase {
  protected:
    SpinWidgetPtr GetSpinWidget() {
        return ReadRealNode<SpinWidget>(
            R"(children: [<"nodes/Widgets/SpinWidget.emd">])", "SpinWidget");
    }
};

TEST_F(CompositeWidgetTest, GetSubWidget) {
    auto sw = GetSpinWidget();

    // Direct sub-widget
    EXPECT_NOT_NULL(sw->GetSubWidget("Ring"));

    // Nested sub-widget
    EXPECT_NOT_NULL(sw->GetSubWidget("Translator"));

    // Not a sub-widget.
    TEST_ASSERT(sw->GetSubWidget("NoSuchWidget"), "No sub-widget");
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
