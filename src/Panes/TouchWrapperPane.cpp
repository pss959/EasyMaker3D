#include "Panes/TouchWrapperPane.h"

#include "SG/Node.h"
#include "SG/Search.h"
#include "Util/General.h"
#include "Widgets/Widget.h"

bool TouchWrapperPane::CanFocus() const {
    return true;
}

void TouchWrapperPane::AddEnabledWidgets(std::vector<WidgetPtr> &widgets) const {
    // Find and add all Widgets wrapped by this Pane.
    const auto is_widget = [](const SG::Node &n){
        if (const Widget *w = dynamic_cast<const Widget *>(&n))
            return w->IsInteractionEnabled();
        else
            return false;
    };
    TouchWrapperPane *cp = const_cast<TouchWrapperPane *>(this);
    SG::NodePtr tp = Util::CreateTemporarySharedPtr<SG::Node>(cp);
    for (auto &node: SG::FindNodes(tp, is_widget))
        widgets.push_back(Util::CastToDerived<Widget>(node));
}
