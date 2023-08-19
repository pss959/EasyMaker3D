#include "Panes/TouchWrapperPane.h"
#include "Tests/Panes/PaneTestBase.h"
#include "Tests/Testing.h"
#include "Util/Tuning.h"
#include "Widgets/Widget.h"

/// \ingroup Tests
class TouchWrapperPaneTest : public PaneTestBase {
  protected:
    // TouchWrapperPane has no template, so this just creates an instance.
    TouchWrapperPanePtr GetTouchWrapperPane(const Str &contents = "") {
        return ReadRealNode<TouchWrapperPane>(contents, "TouchWrapperPane");
    }
};

TEST_F(TouchWrapperPaneTest, GetIntersectedWidget) {
    // TouchWrapperPane has no template, so just create an instance.
    const Str contents = R"(
  children: [
    TouchWrapperPane "TouchWrapperPane" {
      children: [
        PushButtonWidget "But" {},  # Wrapped Widget.
      ]
    }
  ]
)";
    auto tw = GetTouchWrapperPane(contents);
    EXPECT_TRUE(tw->GetPanes().empty());

    auto inter_func = [](const SG::Node &n, float &d){
        d = 10;
        return true;
    };

    float dist = 100;
    auto w = tw->GetIntersectedWidget(inter_func, dist);
    EXPECT_NOT_NULL(w);
    EXPECT_EQ("But", w->GetName());
    EXPECT_EQ(10, dist);
}
